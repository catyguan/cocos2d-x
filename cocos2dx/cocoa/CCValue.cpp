#include "CCValue.h"
#include "CCObject.h"
#include "platform/CCCommon.h"

NS_CC_BEGIN

// CCValue
const CCValue CCValue::nullValue()
{
	CCValue value;
    value.m_type = CCValueTypeNull;
    value.m_field.intValue = 0;
    return value;
}

const CCValue CCValue::intValue(const int intValue)
{
    CCValue value;
    value.m_type = CCValueTypeInt;
    value.m_field.intValue = intValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::numberValue(const double numberValue)
{
    CCValue value;
    value.m_type = CCValueTypeNumber;
    value.m_field.numberValue = numberValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::booleanValue(const bool booleanValue)
{
    CCValue value;
    value.m_type = CCValueTypeBoolean;
    value.m_field.booleanValue = booleanValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::stringValue(const char* stringValue)
{
    CCValue value;
    value.m_type = CCValueTypeString;
    value.m_fieldString = stringValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::stringValue(const std::string& stringValue)
{
    CCValue value;
    value.m_type = CCValueTypeString;
    value.m_fieldString = stringValue;
	value.m_retain = false;
    return value;
}

const CCValue CCValue::mapValue(const CCValueMap& dictValue)
{
    CCValue value;
    value.m_type = CCValueTypeMap;
    value.m_field.mapValue = new CCValueMap(dictValue);
	value.m_retain = false;
    return value;
}

const CCValue CCValue::arrayValue(const CCValueArray& arrayValue)
{
    CCValue value;
    value.m_type = CCValueTypeArray;
    value.m_field.arrayValue = new CCValueArray(arrayValue);
	value.m_retain = false;
    return value;
}

const CCValue CCValue::objectValue(CCObject* obj)
{
	if(obj==NULL)return nullValue();

	CCValue value;
	value.m_type = CCValueTypeObject;
	value.m_field.objectValue = obj;
	value.m_retain = false;
	return value;
}

const CCValue CCValue::fcallValue(CC_FUNCTION_CALL call)
{
	if(call==NULL)return nullValue();

	CCValue value;
	value.m_type = CCValueTypeFunction;
	value.m_field.fcallValue = call;
	value.m_retain = false;
	return value;
}

const CCValue CCValue::ocallValue(CCObject* obj, CC_OBJECT_CALL call)
{	
	if(obj==NULL || call==NULL)return nullValue();	

	CCValue value;
	value.m_type = CCValueTypeObjectCall;
	value.m_field.ocallValue.pObject = obj;
	value.m_field.ocallValue.call = call;
	value.m_retain = false;
	return value;
}

CCValue::CCValue(const CCValue& rhs)
{
	m_type = CCValueTypeNull;
    copy(rhs);
}

CCValue& CCValue::operator=(const CCValue& rhs)
{
    if (this != &rhs) copy(rhs);
    return *this;
}

void CCValue::cleanup()
{
	if (m_type == CCValueTypeMap)
    {
		if(m_field.mapValue!=NULL) {
			delete m_field.mapValue;
		}		
		m_field.mapValue = NULL;
    }
    else if (m_type == CCValueTypeArray)
    {
		if(m_field.arrayValue!=NULL) {
			delete m_field.arrayValue;
		}
		m_field.arrayValue = NULL;
    }
	else if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall)
	{
		if(m_retain) {
			if(m_type==CCValueTypeObject) {
				// CCLOG("--- release 1");
				CC_SAFE_RELEASE_NULL(m_field.objectValue);
			}
			else if(m_type==CCValueTypeObjectCall) {
				// CCLOG("--- oc release 1");
				CC_SAFE_RELEASE_NULL(m_field.ocallValue.pObject);
			}
		}
	}
	m_type = CCValueTypeNull;
}

CCValue::~CCValue(void)
{
	cleanup();    
}

void CCValue::copy(const CCValue& rhs)
{
	cleanup();
    memcpy(&m_field, &rhs.m_field, sizeof(m_field));
    m_type = rhs.m_type;
	m_retain = false;
    if (m_type == CCValueTypeString)
    {
        m_fieldString = rhs.m_fieldString;
    }
    else if (m_type == CCValueTypeMap)
    {
        m_field.mapValue = new CCValueMap(*rhs.m_field.mapValue);
    }
    else if (m_type == CCValueTypeArray)
    {
        m_field.arrayValue = new CCValueArray(*rhs.m_field.arrayValue);
    }	
}

void CCValue::retain()
{
	if(m_type == CCValueTypeObject || m_type==CCValueTypeObjectCall)
	{
		if(!m_retain) {
			m_retain = true;
			if(m_type==CCValueTypeObject) {
				// CCLOG("retain 1");
				CC_SAFE_RETAIN(m_field.objectValue);
			}
			else if(m_type==CCValueTypeObjectCall) {
				// CCLOG("retain 1");
				CC_SAFE_RETAIN(m_field.ocallValue.pObject);
			}
		}
	}
}

bool CCValue::canCall() const {
	return m_type==CCValueTypeFunction || m_type==CCValueTypeObjectCall || m_type==CCValueTypeObject;
}

CCValue CCValue::call(CCValueArray& params,bool throwErr)
{
	try {
		if(m_type==CCValueTypeFunction) {
			return m_field.fcallValue(params);
		} else if(m_type==CCValueTypeObjectCall){
			return (m_field.ocallValue.pObject->*m_field.ocallValue.call)(params);
		} else if(m_type==CCValueTypeObject){
			return m_field.objectValue->invoke(params);
		} else {
			char buf[128];
			sprintf(buf,"invalid value type[%d] for call",m_type);
			throw new std::string(buf);
		}
	} catch(std::string* err) {		
		if(throwErr) {
			throw err;
		} else {
			CCLOG("skip call err - %s", err->c_str());
			delete err;
			return nullValue();
		}
	}
}

bool CCValue::callback(std::string err, CCValue result)
{
	if(isNull())return false;
	CCValueArray ps;
	ps.push_back(stringValue(err));
	ps.push_back(result);
	CCValue r = call(ps, false);
	return r.booleanValue();
}

// CCValueBuilder
CCValueBuilder::CCValueBuilder()
{
	m_current = NULL;
	m_parent = NULL;
}

CCValueBuilder::~CCValueBuilder()
{
	if(m_current!=NULL) {
		deleteItem(m_current);
	}
}

CCValueBuilder* CCValueBuilder::beNull()
{
	CCVBItem* c = current();
	c->type = CCValueTypeNull;
	return this;
}

CCValueBuilder* CCValueBuilder::beInt(const int intValue)
{
	CCVBItem* c = current();
	c->type = CCValueTypeInt;
	c->field.intValue = intValue;
	return this;
}

CCValueBuilder* CCValueBuilder::beNumber(const double numberValue)
{
	CCVBItem* c = current();
	c->type = CCValueTypeNumber;
	c->field.numberValue = numberValue;
	return this;
}

CCValueBuilder* CCValueBuilder::beBoolean(const bool booleanValue)
{
	CCVBItem* c = current();
	c->type = CCValueTypeBoolean;
	c->field.booleanValue = booleanValue;
	return this;
}

CCValueBuilder* CCValueBuilder::beString(const char* stringValue)
{
	CCVBItem* c = current();
	c->type = CCValueTypeString;
	c->strval = stringValue;
	return this;
}

CCValueBuilder* CCValueBuilder::beString(const std::string& stringValue)
{
	CCVBItem* c = current();
	c->type = CCValueTypeString;
	c->strval = stringValue;
	return this;
}

CCValueBuilder* CCValueBuilder::beObject(CCObject* obj)
{
	CCVBItem* c = current();
	c->type = CCValueTypeObject;
	c->field.objectValue = obj;
	return this;
}

CCValueBuilder* CCValueBuilder::beFCall(CC_FUNCTION_CALL call)
{
	CCVBItem* c = current();
	c->type = CCValueTypeFunction;
	c->field.fcallValue = call;
	return this;
}

CCValueBuilder* CCValueBuilder::beOCall(CCObject* obj, CC_OBJECT_CALL call)
{
	CCVBItem* c = current();
	c->type = CCValueTypeObjectCall;
	c->field.ocallValue.pObject = obj;
	c->field.ocallValue.call = call;
	return this;
}

CCValueBuilder* CCValueBuilder::mapBegin()
{
	CCVBItem* c = current();
	c->type = CCValueTypeMap;
	push();
	return this;
}

CCValueBuilder* CCValueBuilder::addMap(const char* key)
{
	CC_ASSERT(m_current!=NULL);
	CC_ASSERT(m_parent!=NULL);

	std::map<std::string, void*>::const_iterator old = m_parent->mapval.find(key);
	if(old!=m_parent->mapval.end()) {
		deleteItem((CCVBItem*) old->second);
	}
	m_parent->mapval[key] = m_current;
	m_current = NULL;

	return this;
}

CCValueBuilder* CCValueBuilder::mapEnd()
{
	pop();
	return this;
}

CCValueBuilder* CCValueBuilder::arrayBegin()
{
	CCVBItem* c = current();
	c->type = CCValueTypeArray;
	push();
	return this;
}

CCValueBuilder* CCValueBuilder::addArray()
{
	CC_ASSERT(m_current!=NULL);
	CC_ASSERT(m_parent!=NULL);

	m_parent->arrayval.push_back(m_current);	
	m_current = NULL;

	return this;
}

CCValueBuilder* CCValueBuilder::arrayEnd()
{
	pop();
	return this;
}

void CCValueBuilder::build(CCValue* root)
{
	build(m_current, root);
}

void CCValueBuilder::build(CCVBItem* item, CCValue* pval)
{
	pval->m_retain = false;
	if(item==NULL) {
		pval->m_type = CCValueTypeNull;
		return;
	}
	switch(item->type) {
	case CCValueTypeArray: 
		{
			pval->m_type = CCValueTypeArray;
			CCValueArray* arr = new CCValueArray();
			build(arr);
			pval->m_field.arrayValue = arr;			
		} break;
	case CCValueTypeMap:
		{
			pval->m_type = CCValueTypeMap;
			CCValueMap* map = new CCValueMap();			
			pval->m_field.mapValue = map;

			std::map<std::string, void*>::const_iterator it = item->mapval.begin();
			while(it!=item->mapval.end()) {
				CCValue* pv = mapAddV(map, it->first.c_str());
				build((CCVBItem*) it->second, pv);
				it++;
			}
		} break;
	default:
		{
			pval->m_type = item->type;
			pval->m_field = item->field;
			pval->m_fieldString = item->strval;
		} break;
	}
}



void CCValueBuilder::build(CCValueArray* root)
{
	build(m_current, root);
}

void CCValueBuilder::build(CCVBItem* item, CCValueArray* pval)
{
	if(item==NULL)return;
	if(item->type!=CCValueTypeArray) {
		CCValue* pv = arrayAddV(pval);
		build(pv);
		return;
	}

	std::vector<void*>::const_iterator it = item->arrayval.begin();
	while(it!=item->arrayval.end()) {
		CCValue* pv = arrayAddV(pval);
		build((CCVBItem*) *it, pv);
		it++;
	}
}

CCVBItem* CCValueBuilder::current()
{
	if(m_current==NULL) {
		m_current = new CCVBItem();
		memset(&m_current->field,0,sizeof(CCValueField));
		m_current->parent = m_parent;
	}
	return m_current;
}

void CCValueBuilder::push()
{
	CC_ASSERT(m_current!=NULL);
	m_parent = m_current;
	m_current = NULL;
}

void CCValueBuilder::pop()
{
	CC_ASSERT(m_parent!=NULL);
	m_current = m_parent;
	m_parent = (CCVBItem*) m_current->parent;
}

void CCValueBuilder::deleteItem(CCVBItem* item)
{
	if(item==NULL) {
		return;
	}
	if(item->arrayval.size()>0) {
		std::vector<void*>::const_iterator it = item->arrayval.begin();
		while(it!=item->arrayval.end()) {
			deleteItem((CCVBItem*) it.operator->());
			it++;
		}
		item->arrayval.clear();
	}
	if(item->mapval.size()>0) {
		std::map<std::string, void*>::const_iterator it = item->mapval.begin();
		while(it!=item->mapval.end()) {
			deleteItem((CCVBItem*) it->second);
			it++;
		}
		item->mapval.clear();
	}
	delete item;
}

CCValue* CCValueBuilder::arrayAddV(CCValueArray* arr)
{
	CCValue v;
	arr->push_back(v);
	return &arr->at(arr->size()-1);
}

CCValue* CCValueBuilder::mapAddV(CCValueMap* map, const char* key)
{
	CCValue v;
	map->insert(std::pair<std::string, CCValue>(key,v));
	return &(map->find(key)->second);
}

// CCValueReader
static CCValue nullValue;
CCValueReader::CCValueReader(CCValue* value)
{
	m_value = value;
}

CCValueReader::~CCValueReader()
{

}

CCValue* CCValueReader::value()
{
	return m_value;
}

bool CCValueReader::isMap()
{
	return m_value->isMap();
}

CCValue* CCValueReader::get(const char* name)
{
	const CCValue* r = getNull(name);
	if(r!=NULL)return (CCValue*) r;
	return &nullValue;
}

CCValue* CCValueReader::getNull(const char* name)
{
	if(m_value->isMap()) {
		CCValueMap* map = m_value->mapValue();
		CCValueMapIterator it = map->find(name);
		if(it!=map->end()) {
			const CCValue* r = &(it->second);
			return (CCValue*) r;
		}
	}
	return NULL;
}

void CCValueReader::remove(const char* name)
{
	if(m_value->isMap()) {
		CCValueMap* map = m_value->mapValue();
		std::map<std::string, CCValue>::iterator it = map->find(name);
		if(it!=map->end()) {
			map->erase(it);
		}
	}
}

bool CCValueReader::beMap(const char* name)
{
	const CCValue* v = getNull(name);
	if(v!=NULL && v->isMap()) {
		if(m_value!=NULL) {
			m_stack.push_back(m_value);
		}
		m_value = (CCValue*) v;
		return true;
	}
	return false;
}

bool CCValueReader::beArray(const char* name)
{
	const CCValue* v = getNull(name);
	if(v!=NULL && v->isArray()) {
		if(m_value!=NULL) {
			m_stack.push_back(m_value);
		}
		m_value = (CCValue*) v;
		return true;
	}
	return false;
}

bool CCValueReader::isArray()
{
	return m_value->isArray();
}

int CCValueReader::arraySize()
{
	if(m_value->isArray()) {
		return m_value->arrayValue()->size();
	}
	return 0;
}

CCValue* CCValueReader::getNull(int idx)
{
	if(m_value->isArray()) {
		CCValueArray* a = m_value->arrayValue();
		if(idx<(int) a->size()) {
			return &a->at(idx);
		}
	}
	return NULL;
}
CCValue* CCValueReader::get(int idx)
{
	CCValue* r = getNull(idx);
	if(r!=NULL)return r;
	return &nullValue;
}

bool CCValueReader::beMap(int idx)
{
	const CCValue* v = getNull(idx);
	if(v!=NULL && v->isMap()) {
		if(m_value!=NULL) {
			m_stack.push_back(m_value);
		}
		m_value = (CCValue*) v;
		return true;
	}
	return false;
}

bool CCValueReader::beArray(int idx)
{
	const CCValue* v = getNull(idx);
	if(v!=NULL && v->isArray()) {
		if(m_value!=NULL) {
			m_stack.push_back(m_value);
		}
		m_value = (CCValue*) v;
		return true;
	}
	return false;
}

void CCValueReader::pop()
{
	if(m_stack.size()>0) {
		std::vector<CCValue*>::iterator it = m_stack.end();
		it--;
		m_value = *it;
		m_stack.erase(it);
	} else {
		CC_ASSERT(false);
	}
}

NS_CC_END