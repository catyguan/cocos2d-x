#include "CCNode_Events.h"
#include "cocoa\CCValueSupport.h"

NS_CC_BEGIN

CCValue CCPropertyChangeEvent::toValue()
{
	CCValueMap map;
	map["name"] = CCValue::stringValue(m_name);
	map["value"] = m_value;
	return CCValue::mapValue(map);
}

CCValue CCMoveEvent::toValue()
{
	CCValueMap map;
	map["position"] = CCValueUtil::point(m_pos.x, m_pos.y);
	map["from"] = CCValueUtil::point(m_from.x,m_from.y);
	return CCValue::mapValue(map);
}

CCValue CCResizeEvent::toValue()
{
	CCValueMap map;
	map["size"] = CCValueUtil::size(m_size.width, m_size.height);
	map["from"] = CCValueUtil::point(m_from.width,m_from.height);
	return CCValue::mapValue(map);
}

NS_CC_END
