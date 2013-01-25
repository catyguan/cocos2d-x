#ifndef __CCNODE_EVENTS_H__
#define __CCNODE_EVENTS_H__

#include "CCNode.h"

NS_CC_BEGIN

#define NODE_EVENT_PROPERTY_CHANGE "propertyChange"

class CC_DLL CCPropertyChangeEvent : public CCNodeEvent
{
public:
	CCPropertyChangeEvent(const char* name, CCValue newValue){m_name = name;m_value = newValue;};
	~CCPropertyChangeEvent(){};

	const char* getName(){return m_name.c_str();};
	CCValue& getValue(){return m_value;};

protected:
	std::string m_name;
	CCValue m_value;
};

#define NODE_EVENT_MOVE "move"

class CC_DLL CCMoveEvent : public CCNodeEvent
{
public:
	CCMoveEvent(CCPoint& pos, CCPoint& from){m_pos = pos;m_from = from;};
	~CCMoveEvent(){};

	CCPoint& getPosition(){return m_pos;};
	CCPoint& getFrom(){return m_from;};

protected:
	CCPoint m_pos;
	CCPoint m_from;
};

NS_CC_END

#endif // __CCNODE_EVENTS_H__


