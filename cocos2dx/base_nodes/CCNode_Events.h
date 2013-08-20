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

	virtual CCValue toValue();

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

	virtual CCValue toValue();

protected:
	CCPoint m_pos;
	CCPoint m_from;
};

#define NODE_EVENT_RESIZE "resize"

class CC_DLL CCResizeEvent : public CCNodeEvent
{
public:
	CCResizeEvent(CCSize& size, CCSize& from){m_size = size;m_from = from;};
	~CCResizeEvent(){};

	CCSize& getSize(){return m_size;};
	CCSize& getFrom(){return m_from;};

	virtual CCValue toValue();

protected:
	CCSize m_size;
	CCSize m_from;
};

#define NODE_EVENT_ENTER "enter"
#define NODE_EVENT_EXIT	 "exit"

NS_CC_END

#endif // __CCNODE_EVENTS_H__


