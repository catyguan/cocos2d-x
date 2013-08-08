#include "CCValueSupport.h"
#include "CCZone.h"

NS_CC_BEGIN

CCCommandObject::CCCommandObject()
{
	m_pObject = NULL;
}

CCCommandObject::~CCCommandObject()
{
	CC_SAFE_RELEASE_NULL(m_pObject);
	m_Params.clear();
}

void CCCommandObject::cleanup()
{
	CC_SAFE_RELEASE_NULL(m_pObject);
	m_Params.clear();	
}

void CCCommandObject::initObjectCommand(CCObject* p,const char* n,CCValueArray& ps)
{
	CC_ASSERT(p);
	m_pObject = p;
	m_pObject->retain();
	m_csName = n;
	m_Params = ps;	
}

CCValue CCCommandObject::invoke(CCValueArray& ps) {
	if(m_pObject!=NULL) {
		CCValueArray cps;
		CCValueUtil::append(cps, m_Params);
		CCValueUtil::append(cps, ps);
		m_pObject->call(m_csName.size()>0?m_csName.c_str():NULL, cps);
	}
	return CCValue::nullValue();
}

CCCommandObject* CCCommandObject::create(CCObject* p, const char* name)
{
	CCValueArray ps;
	return create(p,name,ps);
}

CCCommandObject* CCCommandObject::create(CCObject* p, const char* name, CCValueArray& ctx)
{
	if(p==NULL)return NULL;
	CCCommandObject* r = new CCCommandObject();
	r->initObjectCommand(p,name,ctx);
	r->autorelease();
	return r;
}

// CCValueUtil
void CCValueUtil::append(CCValueArray& r,CCValueArray& a)
{
	if(a.size()>0) {
		for(CCValueArrayIterator it=a.begin();it!=a.end();it++) {
			r.push_back(*it);
		}
	}
}

CCSize CCValueUtil::size(CCValue& v)
{
	CCValueMap* map = v.mapValue();
	if(map!=NULL) {
		float w = 0, h = 0;
		CCValueMapIterator it;
		it = map->find("width");
		if(it!=map->end()) {
			w = it->second.floatValue();
		}
		it = map->find("height");
		if(it!=map->end()) {
			h = it->second.floatValue();
		}
		return CCSizeMake(w, h);
	}
	return CCSizeMake(0,0);
}

CCValue CCValueUtil::size(float w, float h)
{
	CCValueMap map;
	map["width"] = CCValue::numberValue(w);
	map["height"] = CCValue::numberValue(h);
	return CCValue::mapValue(map);
}

CCPoint CCValueUtil::point(CCValue& v)
{
	CCValueMap* map = v.mapValue();
	if(map!=NULL) {
		float x = 0, y = 0;
		CCValueMapIterator it;
		it = map->find("x");
		if(it!=map->end()) {
			x = it->second.floatValue();
		}
		it = map->find("y");
		if(it!=map->end()) {
			y = it->second.floatValue();
		}
		return CCPointMake(x, y);
	}
	return CCPointZero;
}

CCValue CCValueUtil::point(float x, float y)
{
	CCValueMap map;
	map["x"] = CCValue::numberValue(x);
	map["y"] = CCValue::numberValue(y);
	return CCValue::mapValue(map);
}

ccColor4B CCValueUtil::color4b(CCValue& v)
{
	GLubyte r = 0, g = 0, b = 0, o = 255;
	CCValueMap* map = v.mapValue();
	if(map!=NULL) {		
		CCValueMapIterator it;
		it = map->find("r");
		if(it!=map->end()) {
			r = it->second.intValue() & 0xFF;
		}
		it = map->find("g");
		if(it!=map->end()) {
			g = it->second.intValue() & 0xFF;
		}
		it = map->find("b");
		if(it!=map->end()) {
			b = it->second.intValue() & 0xFF;
		}
		it = map->find("o");
		if(it!=map->end()) {
			o = it->second.intValue() & 0xFF;
		}		
	}
	return ccc4(r,g,b,o);
}

ccColor3B CCValueUtil::color3b(CCValue& v)
{
	GLubyte r = 0, g = 0, b = 0;
	CCValueMap* map = v.mapValue();
	if(map!=NULL) {		
		CCValueMapIterator it;
		it = map->find("r");
		if(it!=map->end()) {
			r = it->second.intValue() & 0xFF;
		}
		it = map->find("g");
		if(it!=map->end()) {
			g = it->second.intValue() & 0xFF;
		}
		it = map->find("b");
		if(it!=map->end()) {
			b = it->second.intValue() & 0xFF;
		}		
	}
	return ccc3(r,g,b);
}

CCValue CCValueUtil::color3b(ccColor3B& v)
{
	CCValueMap map;
	map["r"] = CCValue::intValue(v.r);
	map["g"] = CCValue::intValue(v.g);
	map["b"] = CCValue::intValue(v.b);
	return CCValue::mapValue(map);
}

NS_CC_END