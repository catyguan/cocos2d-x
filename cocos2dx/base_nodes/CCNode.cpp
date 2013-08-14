/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Valentin Milea
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "cocoa/CCString.h"
#include "CCNode.h"
#include "support/CCPointExtension.h"
#include "support/TransformUtils.h"
#include "CCCamera.h"
#include "effects/CCGrid.h"
#include "CCDirector.h"
#include "CCScheduler.h"
#include "touch_dispatcher/CCTouch.h"
#include "actions/CCActionManager.h"
#include "shaders/CCGLProgram.h"
// externals
#include "kazmath/GL/matrix.h"
#include "support/component/CCComponent.h"
#include "support/component/CCComponentContainer.h"

// catyguan
#include "cocoa/CCValueSupport.h"
#include "CCNode_Events.h"

#if CC_NODE_RENDER_SUBPIXEL
#define RENDER_IN_SUBPIXEL
#else
#define RENDER_IN_SUBPIXEL(__ARGS__) (ceil(__ARGS__))
#endif

NS_CC_BEGIN

// XXX: Yes, nodes might have a sort problem once every 15 days if the game runs at 60 FPS and each frame sprites are reordered.
static int s_globalOrderOfArrival = 1;

CCNode::CCNode(void)
: m_fRotationX(0.0f)
, m_fRotationY(0.0f)
, m_fScaleX(1.0f)
, m_fScaleY(1.0f)
, m_fVertexZ(0.0f)
, m_obPosition(CCPointZero)
, m_fSkewX(0.0f)
, m_fSkewY(0.0f)
, m_obAnchorPointInPoints(CCPointZero)
, m_obAnchorPoint(CCPointZero)
, m_obContentSize(CCSizeZero)
, m_sAdditionalTransform(CCAffineTransformMakeIdentity())
, m_pCamera(NULL)
// children (lazy allocs)
// lazy alloc
, m_pGrid(NULL)
, m_nZOrder(0)
, m_pChildren(NULL)
, m_pParent(NULL)
// "whole screen" objects. like Scenes and Layers, should set m_bIgnoreAnchorPointForPosition to true
, m_nTag(kCCNodeTagInvalid)
// userData is always inited as nil
, m_pUserData(NULL)
, m_pUserObject(NULL)
, m_pShaderProgram(NULL)
, m_eGLServerState(ccGLServerState(0))
, m_uOrderOfArrival(0)
, m_bRunning(false)
, m_bTransformDirty(true)
, m_bInverseDirty(true)
, m_bAdditionalTransformDirty(false)
, m_bVisible(true)
, m_bIgnoreAnchorPointForPosition(false)
, m_bReorderChildDirty(false)
, m_pAttributes(NULL)
, m_pEventHandlers(NULL)
, m_pMethods(NULL)
, m_pComponentContainer(NULL)
{
    // set default scheduler and actionManager
    CCDirector *director = CCDirector::sharedDirector();
    m_pActionManager = director->getActionManager();
    m_pActionManager->retain();
    m_pScheduler = director->getScheduler();
    m_pScheduler->retain();
    m_pComponentContainer = new CCComponentContainer(this);
}

CCNode::~CCNode(void)
{
    CCLOGINFO( "cocos2d: deallocing" );
    
    CC_SAFE_RELEASE(m_pActionManager);
    CC_SAFE_RELEASE(m_pScheduler);
    // attributes
    CC_SAFE_RELEASE(m_pCamera);

    CC_SAFE_RELEASE(m_pGrid);
    CC_SAFE_RELEASE(m_pShaderProgram);
    CC_SAFE_RELEASE(m_pUserObject);

    if(m_pChildren && m_pChildren->count() > 0)
    {
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
        {
            CCNode* pChild = (CCNode*) child;
            if (pChild)
            {
                pChild->m_pParent = NULL;
            }
        }
    }

    // children
    CC_SAFE_RELEASE(m_pChildren);
    
          // m_pComsContainer
    m_pComponentContainer->removeAll();
    CC_SAFE_DELETE(m_pComponentContainer);

    // catyguan
	removeAllMethods();
	CC_SAFE_DELETE(m_pMethods);
	clearEventHandlers();
	CC_SAFE_DELETE(m_pEventHandlers);
	CC_SAFE_DELETE(m_pAttributes);
}

bool CCNode::init()
{
    return true;
}

bool CCNode::setup(CCValue& properties)
{
	if(properties.isMap()) {
		CCValueMap* map = properties.mapValue();
		CCValueMapIterator it = map->begin();
		for(;it!=map->end();it++) {
			std::string name = it->first;
			if(name.compare("children")==0) {
				if(it->second.isObject()) {
					CCObject* obj = it->second.objectValue();
					if(obj!=NULL) {
						CCNode* node = dynamic_cast<CCNode*>(obj);
						if(node!=NULL) {
							addChild(node);
						}
					}
				} else if(it->second.isArray()) {
					CCValueArray* arr = it->second.arrayValue();
					CCValueArrayIterator it2 = arr->begin();
					for(;it2!=arr->end();it2++) {
						CCObject* obj = it2->objectValue();
						if(obj!=NULL) {
							CCNode* node = dynamic_cast<CCNode*>(obj);
							if(node!=NULL) {
								addChild(node);
							}
						}
					}					
				}
			} else if(name.compare("methods")==0) {
				const CCValue& ms = it->second;
				if(ms.isMap()) {
					CCValueMap* map2 = ms.mapValue();
					CCValueMapIterator it2 = map2->begin();
					for(;it2!=map2->end();it2++) {
						if(it2->second.canCall()) {
							bindMethod(it2->first.c_str(), it2->second);
						}
					}
				}
			} else if(name.compare("layout")==0) {
				const CCValue& ms = it->second;
				if(ms.isMap()) {
					std::string p("layout_");
					CCValueMap* map2 = ms.mapValue();
					CCValueMapIterator it2 = map2->begin();
					for(;it2!=map2->end();it2++) {
						std::string id = p+it2->first;
						this->attribute(id.c_str(), it2->second);
					}
				}
			} else if(name.compare("attributes")==0) {
				const CCValue& ms = it->second;
				if(ms.isMap()) {
					CCValueMap* map2 = ms.mapValue();
					CCValueMapIterator it2 = map2->begin();
					for(;it2!=map2->end();it2++) {
						this->attribute(it2->first.c_str(), it2->second);
					}
				}
			} else if(name.compare("onEvent")==0) {
				const CCValue& ms = it->second;
				if(ms.isMap()) {
					CCValueMap* map2 = ms.mapValue();
					CCValueMapIterator it2 = map2->begin();
					for(;it2!=map2->end();it2++) {
						if(it2->second.canCall()) {
							std::string id = "setup_"+it2->first;
							onEvent(it2->first.c_str(), id.c_str(), it2->second);
						}
					}
				}
			} else {
				if(canCall(name.c_str())) {
					CCValueArray ps;
					ps.push_back(it->second);
					call(it->first.c_str(), ps);
				}
			}
		}
	}
	return true;
}

float CCNode::getSkewX()
{
    return m_fSkewX;
}

void CCNode::setSkewX(float newSkewX)
{
    m_fSkewX = newSkewX;
    m_bTransformDirty = m_bInverseDirty = true;
}

float CCNode::getSkewY()
{
    return m_fSkewY;
}

void CCNode::setSkewY(float newSkewY)
{
    m_fSkewY = newSkewY;

    m_bTransformDirty = m_bInverseDirty = true;
}

/// zOrder getter
int CCNode::getZOrder()
{
    return m_nZOrder;
}

/// zOrder setter : private method
/// used internally to alter the zOrder variable. DON'T call this method manually 
void CCNode::_setZOrder(int z)
{
    m_nZOrder = z;
}

void CCNode::setZOrder(int z)
{
    _setZOrder(z);
    if (m_pParent)
    {
        m_pParent->reorderChild(this, z);
    }
}

/// vertexZ getter
float CCNode::getVertexZ()
{
    return m_fVertexZ;
}


/// vertexZ setter
void CCNode::setVertexZ(float var)
{
    m_fVertexZ = var;
}


/// rotation getter
float CCNode::getRotation()
{
    CCAssert(m_fRotationX == m_fRotationY, "CCNode#rotation. RotationX != RotationY. Don't know which one to return");
    return m_fRotationX;
}

/// rotation setter
void CCNode::setRotation(float newRotation)
{
    m_fRotationX = m_fRotationY = newRotation;
    m_bTransformDirty = m_bInverseDirty = true;
}

float CCNode::getRotationX()
{
    return m_fRotationX;
}

void CCNode::setRotationX(float fRotationX)
{
    m_fRotationX = fRotationX;
    m_bTransformDirty = m_bInverseDirty = true;
}

float CCNode::getRotationY()
{
    return m_fRotationY;
}

void CCNode::setRotationY(float fRotationY)
{
    m_fRotationY = fRotationY;
    m_bTransformDirty = m_bInverseDirty = true;
}

/// scale getter
float CCNode::getScale(void)
{
    CCAssert( m_fScaleX == m_fScaleY, "CCNode#scale. ScaleX != ScaleY. Don't know which one to return");
    return m_fScaleX;
}

/// scale setter
void CCNode::setScale(float scale)
{
    m_fScaleX = m_fScaleY = scale;
    m_bTransformDirty = m_bInverseDirty = true;
}

/// scaleX getter
float CCNode::getScaleX()
{
    return m_fScaleX;
}

/// scaleX setter
void CCNode::setScaleX(float newScaleX)
{
    m_fScaleX = newScaleX;
    m_bTransformDirty = m_bInverseDirty = true;
}

/// scaleY getter
float CCNode::getScaleY()
{
    return m_fScaleY;
}

/// scaleY setter
void CCNode::setScaleY(float newScaleY)
{
    m_fScaleY = newScaleY;
    m_bTransformDirty = m_bInverseDirty = true;
}

/// position getter
const CCPoint& CCNode::getPosition()
{
    return m_obPosition;
}

/// position setter
void CCNode::setPosition(const CCPoint& newPosition)
{
	CCPoint old = m_obPosition;
    m_obPosition = newPosition;
    m_bTransformDirty = m_bInverseDirty = true;
	if(!old.equals(newPosition) && hasEventHandler(NODE_EVENT_MOVE)) {
		CCMoveEvent e(m_obPosition, old);
		raiseEvent(NODE_EVENT_MOVE, &e);
	}
}

void CCNode::getPosition(float* x, float* y)
{
    *x = m_obPosition.x;
    *y = m_obPosition.y;
}

void CCNode::setPosition(float x, float y)
{
    setPosition(ccp(x, y));
}

float CCNode::getPositionX(void)
{
    return m_obPosition.x;
}

float CCNode::getPositionY(void)
{
    return  m_obPosition.y;
}

void CCNode::setPositionX(float x)
{
    setPosition(ccp(x, m_obPosition.y));
}

void CCNode::setPositionY(float y)
{
    setPosition(ccp(m_obPosition.x, y));
}

/// children getter
CCArray* CCNode::getChildren()
{
    return m_pChildren;
}

unsigned int CCNode::getChildrenCount(void) const
{
    return m_pChildren ? m_pChildren->count() : 0;
}

/// camera getter: lazy alloc
CCCamera* CCNode::getCamera()
{
    if (!m_pCamera)
    {
        m_pCamera = new CCCamera();
    }
    
    return m_pCamera;
}


/// grid getter
CCGridBase* CCNode::getGrid()
{
    return m_pGrid;
}

/// grid setter
void CCNode::setGrid(CCGridBase* pGrid)
{
    CC_SAFE_RETAIN(pGrid);
    CC_SAFE_RELEASE(m_pGrid);
    m_pGrid = pGrid;
}


/// isVisible getter
bool CCNode::isVisible()
{
    return m_bVisible;
}

/// isVisible setter
void CCNode::setVisible(bool var)
{
    m_bVisible = var;
}

const CCPoint& CCNode::getAnchorPointInPoints()
{
    return m_obAnchorPointInPoints;
}

/// anchorPoint getter
const CCPoint& CCNode::getAnchorPoint()
{
    return m_obAnchorPoint;
}

void CCNode::setAnchorPoint(const CCPoint& point)
{
    if( ! point.equals(m_obAnchorPoint))
    {
        m_obAnchorPoint = point;
        m_obAnchorPointInPoints = ccp(m_obContentSize.width * m_obAnchorPoint.x, m_obContentSize.height * m_obAnchorPoint.y );
        m_bTransformDirty = m_bInverseDirty = true;
    }
}

/// contentSize getter
const CCSize& CCNode::getContentSize() const
{
    return m_obContentSize;
}

void CCNode::setContentSize(const CCSize & size)
{
    if ( ! size.equals(m_obContentSize))
    {
        m_obContentSize = size;

        m_obAnchorPointInPoints = ccp(m_obContentSize.width * m_obAnchorPoint.x, m_obContentSize.height * m_obAnchorPoint.y );
        m_bTransformDirty = m_bInverseDirty = true;
    }
}

// isRunning getter
bool CCNode::isRunning()
{
    return m_bRunning;
}

/// parent getter
CCNode * CCNode::getParent()
{
    return m_pParent;
}
/// parent setter
void CCNode::setParent(CCNode * var)
{
    m_pParent = var;
}

/// isRelativeAnchorPoint getter
bool CCNode::isIgnoreAnchorPointForPosition()
{
    return m_bIgnoreAnchorPointForPosition;
}
/// isRelativeAnchorPoint setter
void CCNode::ignoreAnchorPointForPosition(bool newValue)
{
    if (newValue != m_bIgnoreAnchorPointForPosition) 
    {
		m_bIgnoreAnchorPointForPosition = newValue;
		m_bTransformDirty = m_bInverseDirty = true;
	}
}

/// tag getter
int CCNode::getTag() const
{
    return m_nTag;
}

/// tag setter
void CCNode::setTag(int var)
{
    m_nTag = var;
}

/// id setter
std::string CCNode::getId()
{
    return m_nId;
}

/// tag setter
void CCNode::setId(const char* id)
{
    m_nId = id;
}


/// userData getter
void * CCNode::getUserData()
{
    return m_pUserData;
}

/// userData setter
void CCNode::setUserData(void *var)
{
    m_pUserData = var;
}

unsigned int CCNode::getOrderOfArrival()
{
    return m_uOrderOfArrival;
}

void CCNode::setOrderOfArrival(unsigned int uOrderOfArrival)
{
    m_uOrderOfArrival = uOrderOfArrival;
}

CCGLProgram* CCNode::getShaderProgram()
{
    return m_pShaderProgram;
}

CCObject* CCNode::getUserObject()
{
    return m_pUserObject;
}

ccGLServerState CCNode::getGLServerState()
{
    return m_eGLServerState;
}

void CCNode::setGLServerState(ccGLServerState glServerState)
{
    m_eGLServerState = glServerState;
}

void CCNode::setUserObject(CCObject *pUserObject)
{
    CC_SAFE_RELEASE(m_pUserObject);
    CC_SAFE_RETAIN(pUserObject);
    m_pUserObject = pUserObject;
}

void CCNode::setShaderProgram(CCGLProgram *pShaderProgram)
{
    CC_SAFE_RELEASE(m_pShaderProgram);
    m_pShaderProgram = pShaderProgram;
    CC_SAFE_RETAIN(m_pShaderProgram);
}

CCRect CCNode::boundingBox()
{
    CCRect rect = CCRectMake(0, 0, m_obContentSize.width, m_obContentSize.height);
    return CCRectApplyAffineTransform(rect, nodeToParentTransform());
}

CCNode * CCNode::create(void)
{
	CCNode * pRet = new CCNode();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(pRet);
    }
	return pRet;
}

void CCNode::cleanup()
{
    // actions
    this->stopAllActions();
    this->unscheduleAllSelectors();    

    // timers
    arrayMakeObjectsPerformSelector(m_pChildren, cleanup, CCNode*);

	clearEventHandlers();
	CC_SAFE_DELETE(m_pEventHandlers);
	m_pEventHandlers = NULL;

	CC_SAFE_DELETE(m_pAttributes);
	m_pAttributes = NULL;
}


const char* CCNode::description()
{
    return CCString::createWithFormat("<CCNode | Tag = %d>", m_nTag)->getCString();
}

// lazy allocs
void CCNode::childrenAlloc(void)
{
    m_pChildren = CCArray::createWithCapacity(4);
    m_pChildren->retain();
}

CCNode* CCNode::getChildByTag(int aTag)
{
    CCAssert( aTag != kCCNodeTagInvalid, "Invalid tag");

    if(m_pChildren && m_pChildren->count() > 0)
    {
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
        {
            CCNode* pNode = (CCNode*) child;
            if(pNode && pNode->m_nTag == aTag)
                return pNode;
        }
    }
    return NULL;
}

CCNode* CCNode::getChildById(const char* id)
{
    CCAssert( id != NULL, "Invalid id");

    if(m_pChildren && m_pChildren->count() > 0)
    {
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
        {
            CCNode* pNode = (CCNode*) child;
			if(pNode && pNode->m_nId.compare(id) == 0)
                return pNode;
        }
    }
    return NULL;
}

CCObject* CCNode::findChildById(const char* id)
{
	return getChildById(id);
}

/* "add" logic MUST only be on this method
* If a class want's to extend the 'addChild' behavior it only needs
* to override this method
*/
void CCNode::addChild(CCNode *child, int zOrder, int tag)
{    
    CCAssert( child != NULL, "Argument must be non-nil");
    CCAssert( child->m_pParent == NULL, "child already added. It can't be added again");

    if( ! m_pChildren )
    {
        this->childrenAlloc();
    }

    this->insertChild(child, zOrder);

    child->m_nTag = tag;

    child->setParent(this);
    child->setOrderOfArrival(s_globalOrderOfArrival++);

    if( m_bRunning )
    {
        child->onEnter();
        child->onEnterTransitionDidFinish();
    }
}

void CCNode::addChild(CCNode *child, int zOrder)
{
    CCAssert( child != NULL, "Argument must be non-nil");
    this->addChild(child, zOrder, child->m_nTag);
}

void CCNode::addChild(CCNode *child)
{
    CCAssert( child != NULL, "Argument must be non-nil");
    this->addChild(child, child->m_nZOrder, child->m_nTag);
}

void CCNode::removeFromParent()
{
    this->removeFromParentAndCleanup(true);
}

void CCNode::removeFromParentAndCleanup(bool cleanup)
{
    if (m_pParent != NULL)
    {
        m_pParent->removeChild(this,cleanup);
    } 
}

void CCNode::removeChild(CCNode* child)
{
    this->removeChild(child, true);
}

/* "remove" logic MUST only be on this method
* If a class want's to extend the 'removeChild' behavior it only needs
* to override this method
*/
void CCNode::removeChild(CCNode* child, bool cleanup)
{
    // explicit nil handling
    if (m_pChildren == NULL)
    {
        return;
    }

    if ( m_pChildren->containsObject(child) )
    {
        this->detachChild(child,cleanup);
    }
}

void CCNode::removeChildByTag(int tag)
{
    this->removeChildByTag(tag, true);
}

void CCNode::removeChildByTag(int tag, bool cleanup)
{
    CCAssert( tag != kCCNodeTagInvalid, "Invalid tag");

    CCNode *child = this->getChildByTag(tag);

    if (child == NULL)
    {
        CCLOG("cocos2d: removeChildByTag(tag = %d): child not found!", tag);
    }
    else
    {
        this->removeChild(child, cleanup);
    }
}

void CCNode::removeChildById(const char* id)
{
    this->removeChildById(id, true);
}

void CCNode::removeChildById(const char* id, bool cleanup)
{
    CCAssert( id != NULL, "Invalid id");

    CCNode *child = this->getChildById(id);

    if (child == NULL)
    {
        CCLOG("cocos2d: removeChildByTag: child not found!");
    }
    else
    {
        this->removeChild(child, cleanup);
    }
}

void CCNode::removeAllChildren()
{
    this->removeAllChildrenWithCleanup(true);
}

void CCNode::removeAllChildrenWithCleanup(bool cleanup)
{
    // not using detachChild improves speed here
    if ( m_pChildren && m_pChildren->count() > 0 )
    {
        CCObject* child;
        CCARRAY_FOREACH(m_pChildren, child)
        {
            CCNode* pNode = (CCNode*) child;
            if (pNode)
            {
                // IMPORTANT:
                //  -1st do onExit
                //  -2nd cleanup
                if(m_bRunning)
                {
                    pNode->onExitTransitionDidStart();
                    pNode->onExit();
                }

                if (cleanup)
                {
                    pNode->cleanup();
                }
                // set parent nil at the end
                pNode->setParent(NULL);
            }
        }
        
        m_pChildren->removeAllObjects();
    }
    
}

void CCNode::detachChild(CCNode *child, bool doCleanup)
{
    // IMPORTANT:
    //  -1st do onExit
    //  -2nd cleanup
    if (m_bRunning)
    {
        child->onExitTransitionDidStart();
        child->onExit();
    }

    // If you don't do cleanup, the child's actions will not get removed and the
    // its scheduledSelectors_ dict will not get released!
    if (doCleanup)
    {
        child->cleanup();
    }

    // set parent nil at the end
    child->setParent(NULL);

    m_pChildren->removeObject(child);
}


// helper used by reorderChild & add
void CCNode::insertChild(CCNode* child, int z)
{
    m_bReorderChildDirty = true;
    ccArrayAppendObjectWithResize(m_pChildren->data, child);
    child->_setZOrder(z);
}

void CCNode::reorderChild(CCNode *child, int zOrder)
{
    CCAssert( child != NULL, "Child must be non-nil");
    m_bReorderChildDirty = true;
    child->setOrderOfArrival(s_globalOrderOfArrival++);
    child->_setZOrder(zOrder);
}

void CCNode::sortAllChildren()
{
    if (m_bReorderChildDirty)
    {
        int i,j,length = m_pChildren->data->num;
        CCNode ** x = (CCNode**)m_pChildren->data->arr;
        CCNode *tempItem;

        // insertion sort
        for(i=1; i<length; i++)
        {
            tempItem = x[i];
            j = i-1;

            //continue moving element downwards while zOrder is smaller or when zOrder is the same but mutatedIndex is smaller
            while(j>=0 && ( tempItem->m_nZOrder < x[j]->m_nZOrder || ( tempItem->m_nZOrder== x[j]->m_nZOrder && tempItem->m_uOrderOfArrival < x[j]->m_uOrderOfArrival ) ) )
            {
                x[j+1] = x[j];
                j = j-1;
            }
            x[j+1] = tempItem;
        }

        //don't need to check children recursively, that's done in visit of each child

        m_bReorderChildDirty = false;
    }
}


 void CCNode::draw()
 {
     //CCAssert(0);
     // override me
     // Only use- this function to draw your stuff.
     // DON'T draw your stuff outside this method
 }

void CCNode::visit()
{
    // quick return if not visible. children won't be drawn.
    if (!m_bVisible)
    {
        return;
    }
    kmGLPushMatrix();

     if (m_pGrid && m_pGrid->isActive())
     {
         m_pGrid->beforeDraw();
     }

    this->transform();

    CCNode* pNode = NULL;
    unsigned int i = 0;

    if(m_pChildren && m_pChildren->count() > 0)
    {
        sortAllChildren();
        // draw children zOrder < 0
        ccArray *arrayData = m_pChildren->data;
        for( ; i < arrayData->num; i++ )
        {
            pNode = (CCNode*) arrayData->arr[i];

            if ( pNode && pNode->m_nZOrder < 0 ) 
            {
                pNode->visit();
            }
            else
            {
                break;
            }
        }
        // self draw
        this->draw();

        for( ; i < arrayData->num; i++ )
        {
            pNode = (CCNode*) arrayData->arr[i];
            if (pNode)
            {
                pNode->visit();
            }
        }        
    }
    else
    {
        this->draw();
    }

    // reset for next frame
    m_uOrderOfArrival = 0;

     if (m_pGrid && m_pGrid->isActive())
     {
         m_pGrid->afterDraw(this);
    }
 
    kmGLPopMatrix();
}

void CCNode::transformAncestors()
{
    if( m_pParent != NULL  )
    {
        m_pParent->transformAncestors();
        m_pParent->transform();
    }
}

void CCNode::transform()
{    
    kmMat4 transfrom4x4;

    // Convert 3x3 into 4x4 matrix
    CCAffineTransform tmpAffine = this->nodeToParentTransform();
    CGAffineToGL(&tmpAffine, transfrom4x4.mat);

    // Update Z vertex manually
    transfrom4x4.mat[14] = m_fVertexZ;

    kmGLMultMatrix( &transfrom4x4 );


    // XXX: Expensive calls. Camera should be integrated into the cached affine matrix
    if ( m_pCamera != NULL && !(m_pGrid != NULL && m_pGrid->isActive()) )
    {
        bool translate = (m_obAnchorPointInPoints.x != 0.0f || m_obAnchorPointInPoints.y != 0.0f);

        if( translate )
            kmGLTranslatef(RENDER_IN_SUBPIXEL(m_obAnchorPointInPoints.x), RENDER_IN_SUBPIXEL(m_obAnchorPointInPoints.y), 0 );

        m_pCamera->locate();

        if( translate )
            kmGLTranslatef(RENDER_IN_SUBPIXEL(-m_obAnchorPointInPoints.x), RENDER_IN_SUBPIXEL(-m_obAnchorPointInPoints.y), 0 );
    }

}


void CCNode::onEnter()
{
    arrayMakeObjectsPerformSelector(m_pChildren, onEnter, CCNode*);

    this->resumeSchedulerAndActions();

    m_bRunning = true;

	raiseEvent("enter",NULL);	
}

void CCNode::onEnterTransitionDidFinish()
{
    arrayMakeObjectsPerformSelector(m_pChildren, onEnterTransitionDidFinish, CCNode*);
}

void CCNode::onExitTransitionDidStart()
{
    arrayMakeObjectsPerformSelector(m_pChildren, onExitTransitionDidStart, CCNode*);
}

void CCNode::onExit()
{
    this->pauseSchedulerAndActions();

    m_bRunning = false;

    arrayMakeObjectsPerformSelector(m_pChildren, onExit, CCNode*);    
	
	raiseEvent("exit",NULL);	
}

void CCNode::setActionManager(CCActionManager* actionManager)
{
    if( actionManager != m_pActionManager ) {
        this->stopAllActions();
        CC_SAFE_RETAIN(actionManager);
        CC_SAFE_RELEASE(m_pActionManager);
        m_pActionManager = actionManager;
    }
}

CCActionManager* CCNode::getActionManager()
{
    return m_pActionManager;
}

CCAction * CCNode::runAction(CCAction* action)
{
    CCAssert( action != NULL, "Argument must be non-nil");
    m_pActionManager->addAction(action, this, !m_bRunning);
    return action;
}

void CCNode::stopAllActions()
{
    m_pActionManager->removeAllActionsFromTarget(this);
}

void CCNode::stopAction(CCAction* action)
{
    m_pActionManager->removeAction(action);
}

void CCNode::stopActionByTag(int tag)
{
    CCAssert( tag != kCCActionTagInvalid, "Invalid tag");
    m_pActionManager->removeActionByTag(tag, this);
}

void CCNode::stopActionById(const char* id)
{
    CCAssert( id != NULL, "Invalid id");
    m_pActionManager->removeActionById(id, this);
}

CCAction * CCNode::getActionByTag(int tag)
{
    CCAssert( tag != kCCActionTagInvalid, "Invalid tag");
    return m_pActionManager->getActionByTag(tag, this);
}

CCAction * CCNode::getActionById(const char* id)
{
    CCAssert( id != NULL, "Invalid id");
    return m_pActionManager->getActionById(id, this);
}


unsigned int CCNode::numberOfRunningActions()
{
    return m_pActionManager->numberOfRunningActionsInTarget(this);
}

// CCNode - Callbacks

void CCNode::setScheduler(CCScheduler* scheduler)
{
    if( scheduler != m_pScheduler ) {
        this->unscheduleAllSelectors();
        CC_SAFE_RETAIN(scheduler);
        CC_SAFE_RELEASE(m_pScheduler);
        m_pScheduler = scheduler;
    }
}

CCScheduler* CCNode::getScheduler()
{
    return m_pScheduler;
}

void CCNode::scheduleUpdate()
{
    scheduleUpdateWithPriority(0);
}

void CCNode::scheduleUpdateWithPriority(int priority)
{
    m_pScheduler->scheduleUpdateForTarget(this, priority, !m_bRunning);
}

void CCNode::unscheduleUpdate()
{
    m_pScheduler->unscheduleUpdateForTarget(this);    
}

void CCNode::schedule(SEL_SCHEDULE selector)
{
    this->schedule(selector, 0.0f, kCCRepeatForever, 0.0f);
}

void CCNode::schedule(SEL_SCHEDULE selector, float interval)
{
    this->schedule(selector, interval, kCCRepeatForever, 0.0f);
}

void CCNode::schedule(SEL_SCHEDULE selector, float interval, unsigned int repeat, float delay)
{
    CCAssert( selector, "Argument must be non-nil");
    CCAssert( interval >=0, "Argument must be positive");

    m_pScheduler->scheduleSelector(selector, this, interval , repeat, delay, !m_bRunning);
}

void CCNode::scheduleOnce(SEL_SCHEDULE selector, float delay)
{
    this->schedule(selector, 0.0f, 0, delay);
}

void CCNode::unschedule(SEL_SCHEDULE selector)
{
    // explicit nil handling
    if (selector == 0)
        return;

    m_pScheduler->unscheduleSelector(selector, this);
}

void CCNode::unscheduleAllSelectors()
{
    m_pScheduler->unscheduleAllForTarget(this);
}

void CCNode::resumeSchedulerAndActions()
{
    m_pScheduler->resumeTarget(this);
    m_pActionManager->resumeTarget(this);
}

void CCNode::pauseSchedulerAndActions()
{
    m_pScheduler->pauseTarget(this);
    m_pActionManager->pauseTarget(this);
}

// override me
void CCNode::update(float fDelta)
{
    
    
    if (m_pComponentContainer && !m_pComponentContainer->isEmpty())
    {
        m_pComponentContainer->visit(fDelta);
    }
}

CCAffineTransform CCNode::nodeToParentTransform(void)
{
    if (m_bTransformDirty) 
    {

        // Translate values
        float x = m_obPosition.x;
        float y = m_obPosition.y;

        if (m_bIgnoreAnchorPointForPosition) 
        {
            x += m_obAnchorPointInPoints.x;
            y += m_obAnchorPointInPoints.y;
        }

        // Rotation values
		// Change rotation code to handle X and Y
		// If we skew with the exact same value for both x and y then we're simply just rotating
        float cx = 1, sx = 0, cy = 1, sy = 0;
        if (m_fRotationX || m_fRotationY)
        {
            float radiansX = -CC_DEGREES_TO_RADIANS(m_fRotationX);
            float radiansY = -CC_DEGREES_TO_RADIANS(m_fRotationY);
            cx = cosf(radiansX);
            sx = sinf(radiansX);
            cy = cosf(radiansY);
            sy = sinf(radiansY);
        }

        bool needsSkewMatrix = ( m_fSkewX || m_fSkewY );


        // optimization:
        // inline anchor point calculation if skew is not needed
        // Adjusted transform calculation for rotational skew
        if (! needsSkewMatrix && !m_obAnchorPointInPoints.equals(CCPointZero))
        {
            x += cy * -m_obAnchorPointInPoints.x * m_fScaleX + -sx * -m_obAnchorPointInPoints.y * m_fScaleY;
            y += sy * -m_obAnchorPointInPoints.x * m_fScaleX +  cx * -m_obAnchorPointInPoints.y * m_fScaleY;
        }


        // Build Transform Matrix
        // Adjusted transform calculation for rotational skew
        m_sTransform = CCAffineTransformMake( cy * m_fScaleX,  sy * m_fScaleX,
            -sx * m_fScaleY, cx * m_fScaleY,
            x, y );

        // XXX: Try to inline skew
        // If skew is needed, apply skew and then anchor point
        if (needsSkewMatrix) 
        {
            CCAffineTransform skewMatrix = CCAffineTransformMake(1.0f, tanf(CC_DEGREES_TO_RADIANS(m_fSkewY)),
                tanf(CC_DEGREES_TO_RADIANS(m_fSkewX)), 1.0f,
                0.0f, 0.0f );
            m_sTransform = CCAffineTransformConcat(skewMatrix, m_sTransform);

            // adjust anchor point
            if (!m_obAnchorPointInPoints.equals(CCPointZero))
            {
                m_sTransform = CCAffineTransformTranslate(m_sTransform, -m_obAnchorPointInPoints.x, -m_obAnchorPointInPoints.y);
            }
        }
        
        if (m_bAdditionalTransformDirty)
        {
            m_sTransform = CCAffineTransformConcat(m_sTransform, m_sAdditionalTransform);
            m_bAdditionalTransformDirty = false;
        }

        m_bTransformDirty = false;
    }

    return m_sTransform;
}

void CCNode::setAdditionalTransform(const CCAffineTransform& additionalTransform)
{
    m_sAdditionalTransform = additionalTransform;
    m_bTransformDirty = true;
    m_bAdditionalTransformDirty = true;
}

CCAffineTransform CCNode::parentToNodeTransform(void)
{
    if ( m_bInverseDirty ) {
        m_sInverse = CCAffineTransformInvert(this->nodeToParentTransform());
        m_bInverseDirty = false;
    }

    return m_sInverse;
}

CCAffineTransform CCNode::nodeToWorldTransform()
{
    CCAffineTransform t = this->nodeToParentTransform();

    for (CCNode *p = m_pParent; p != NULL; p = p->getParent())
        t = CCAffineTransformConcat(t, p->nodeToParentTransform());

    return t;
}

CCAffineTransform CCNode::worldToNodeTransform(void)
{
    return CCAffineTransformInvert(this->nodeToWorldTransform());
}

CCPoint CCNode::convertToNodeSpace(const CCPoint& worldPoint)
{
    CCPoint ret = CCPointApplyAffineTransform(worldPoint, worldToNodeTransform());
    return ret;
}

CCPoint CCNode::convertToWorldSpace(const CCPoint& nodePoint)
{
    CCPoint ret = CCPointApplyAffineTransform(nodePoint, nodeToWorldTransform());
    return ret;
}

CCPoint CCNode::convertToNodeSpaceAR(const CCPoint& worldPoint)
{
    CCPoint nodePoint = convertToNodeSpace(worldPoint);
    return ccpSub(nodePoint, m_obAnchorPointInPoints);
}

CCPoint CCNode::convertToWorldSpaceAR(const CCPoint& nodePoint)
{
    CCPoint pt = ccpAdd(nodePoint, m_obAnchorPointInPoints);
    return convertToWorldSpace(pt);
}

CCPoint CCNode::convertToWindowSpace(const CCPoint& nodePoint)
{
    CCPoint worldPoint = this->convertToWorldSpace(nodePoint);
    return CCDirector::sharedDirector()->convertToUI(worldPoint);
}

// convenience methods which take a CCTouch instead of CCPoint
CCPoint CCNode::convertTouchToNodeSpace(CCTouch *touch)
{
    CCPoint point = touch->getLocation();
    return this->convertToNodeSpace(point);
}
CCPoint CCNode::convertTouchToNodeSpaceAR(CCTouch *touch)
{
    CCPoint point = touch->getLocation();
    return this->convertToNodeSpaceAR(point);
}

bool CCNode::containsPoint(CCPoint& point)
{
	CCRect r = boundingBox();
	r.origin = CCPointZero;
	return r.containsPoint(point);
}

void CCNode::updateTransform()
{
   // Recursively iterate over children
    arrayMakeObjectsPerformSelector(m_pChildren, updateTransform, CCNode*);
}

CCComponent* CCNode::getComponent(const char *pName) const
{
    return m_pComponentContainer->get(pName);
}

bool CCNode::addComponent(CCComponent *pComponent)
{
    return m_pComponentContainer->add(pComponent);
}

bool CCNode::removeComponent(const char *pName)
{
    return m_pComponentContainer->remove(pName);
}

void CCNode::removeAllComponents()
{
    m_pComponentContainer->removeAll();
}

// catyguan
#include "../cocoa/CCValueSupport.h"
// cc_call
CC_BEGIN_CALLS(CCNode, CCObject)
	CC_DEFINE_CALL(CCNode, bindMethod)
	CC_DEFINE_CALL(CCNode, removeMethod)
	CC_DEFINE_CALL(CCNode, removeAllMethods)
	CC_DEFINE_CALL(CCNode, setup)
	CC_DEFINE_CALL(CCNode, addChild)
	CC_DEFINE_CALL(CCNode, removeChild)
	CC_DEFINE_CALL(CCNode, runAction)
	CC_DEFINE_CALL(CCNode, stopAllActions)
	CC_DEFINE_CALL(CCNode, stopAction)
	CC_DEFINE_CALL(CCNode, getAction)
	CC_DEFINE_CALL(CCNode, anchorPoint)
	CC_DEFINE_CALL(CCNode, contentSize)
	CC_DEFINE_CALL(CCNode, width)
	CC_DEFINE_CALL(CCNode, height)
	CC_DEFINE_CALL(CCNode, ignoreAnchorPointForPosition)
	CC_DEFINE_CALL(CCNode, rotationX)
	CC_DEFINE_CALL(CCNode, rotationY)
	CC_DEFINE_CALL(CCNode, rotation)
	CC_DEFINE_CALL(CCNode, positionX)
	CC_DEFINE_CALL_ALIAS(CCNode, x, positionX)	
	CC_DEFINE_CALL(CCNode, positionY)
	CC_DEFINE_CALL_ALIAS(CCNode, y, positionY)
	CC_DEFINE_CALL(CCNode, position)
	CC_DEFINE_CALL(CCNode, scaleX)
	CC_DEFINE_CALL(CCNode, scaleY)	
	CC_DEFINE_CALL(CCNode, scale)
	CC_DEFINE_CALL(CCNode, skewX)
	CC_DEFINE_CALL(CCNode, skewY)
	CC_DEFINE_CALL(CCNode, id)
	CC_DEFINE_CALL(CCNode, tag)
	CC_DEFINE_CALL(CCNode, position)
	CC_DEFINE_CALL(CCNode, visible)	
	CC_DEFINE_CALL(CCNode, zOrder)
	CC_DEFINE_CALL(CCNode, onEvent)
	CC_DEFINE_CALL(CCNode, removeEvent)
// CC_END_CALLS(CCNode, CCObject)
{NULL,NULL}};

bool CCNode::methodCanCall(const char* method)
{
	if(m_pMethods!=NULL) {
		CCValueMapIterator it = m_pMethods->find(method);
		if(it!=m_pMethods->end())return true;
	}
	return false;
}

bool CCNode::canCall(const char* method){
	if(CCObject::canCallImpl(this, s_CALLS_CCNode, method)) { return true; }
	if(methodCanCall(method))return true;	
	return CCObject::canCall(method);
}

bool CCNode::methodCall(const char* method, CCValueArray& params,CCValue& r)
{
	if(m_pMethods!=NULL) {
		CCValueMapIterator it = m_pMethods->find(method);
		if(it!=m_pMethods->end()) {
			CCValue call = it->second;
			if(call.canCall()) {
				call.retain();
				params.insert(params.begin(), CCValue::objectValue(this));
				r = call.call(params, false);
				call.cleanup();
				return true;
			}
		}
	}
	return false;
}

CCValue CCNode::call(const char* method, CCValueArray& params){
	if(method==NULL)return invoke(params);
	CCValue r;
	if(CCObject::callImpl(this, s_CALLS_CCNode, r, method, params)) { return r; }
	if(methodCall(method, params, r)) {
		return r;
	}	
	return CCObject::call(method, params);
}

CCValue CCNode::CALLNAME(bindMethod)(CCValueArray& params) {
	std::string name = ccvpString(params,0);
	CCValue call = ccvp(params,1);
	bindMethod(name.c_str(), call);
	return CCValue::nullValue();
}
CCValue CCNode::CALLNAME(removeMethod)(CCValueArray& params) {
	std::string name = ccvpString(params,0);
	removeMethod(name.c_str());
	return CCValue::nullValue();
}
CCValue CCNode::CALLNAME(removeAllMethods)(CCValueArray& params) {
	removeAllMethods();
	return CCValue::nullValue();
}
CCValue CCNode::CALLNAME(setup)(CCValueArray& params) {
	bool r = false;
	if(params.size()>0) {
		r = setup(params[0]);
	} else {
		CCValue tmp;
		r = setup(tmp);
	}
	return CCValue::booleanValue(r);
}
CCValue CCNode::CALLNAME(addChild)(CCValueArray& params) {
	CCNode* ch = ccvpObject(params,0,CCNode);
	int zOrder = ccvpInt(params,1);
	int tag = ccvpInt(params,2);
	bool r = false;
	if(ch!=NULL) {
		addChild(ch, zOrder, tag);
		r = true;
	}
	return CCValue::booleanValue(r);
}
CCValue CCNode::CALLNAME(removeChild)(CCValueArray& params) {
	bool r = false;
	if(params.size()>0) {
		if(params[0].isString()) {
			std::string id = params[0].stringValue();
			removeChildById(id.c_str());
			r = true;
		} else {
			removeChildByTag(params[0].intValue());
			r = true;
		}
	}
	return CCValue::intValue(r);
}
CCValue CCNode::CALLNAME(runAction)(CCValueArray& params) {
	CCAction* obj = ccvpObject(params,0,CCAction);
	runAction(obj);
	return CCValue::booleanValue(true);
}
CCValue CCNode::CALLNAME(stopAllActions)(CCValueArray& params) {
	stopAllActions();
	return CCValue::nullValue();
}
CCValue CCNode::CALLNAME(stopAction)(CCValueArray& params) {
	if(params.size()>0) {
		CCValue& v = params[0];
		if(v.isInt() || v.isNumber()) {
			stopActionByTag(v.intValue());
			return CCValue::booleanValue(true);
		} else if(v.isString()) {
			stopActionById(v.stringValue().c_str());
			return CCValue::booleanValue(true);
		}
	}
	return CCValue::booleanValue(false);
}
CCValue CCNode::CALLNAME(getAction)(CCValueArray& params) {
	if(params.size()>0) {
		CCValue& v = params[0];
		if(v.isInt() || v.isNumber()) {
			return CCValue::objectValue(getActionByTag(v.intValue()));
		} else if(v.isString()) {
			return CCValue::objectValue(getActionById(v.stringValue().c_str()));
		}
	}
	return CCValue::nullValue();
}
CCValue CCNode::CALLNAME(anchorPoint)(CCValueArray& params) {
	if(params.size()>0) {
		CCPoint pt = ccvpPoint(params, 0);
		setAnchorPoint(pt);
	}
	CCPoint pt2 = getAnchorPoint();
	return CCValueUtil::point(pt2.x, pt2.y);
}
CCValue CCNode::CALLNAME(contentSize)(CCValueArray& params) {
	if(params.size()>0) {
		CCSize sz = ccvpSize(params, 0);
		setContentSize(sz);
	}
	CCSize sz2 = getContentSize();
	return CCValueUtil::size(sz2.width, sz2.height);
}
CCValue CCNode::CALLNAME(width)(CCValueArray& params) {
	if(params.size()>0) {
		int v = ccvpInt(params,0);
		CCSize sz = getContentSize();
		setContentSize(CCSizeMake(v, sz.height));
	}
	CCSize sz2 = getContentSize();
	return CCValue::intValue(sz2.width);
}
CCValue CCNode::CALLNAME(height)(CCValueArray& params) {
	if(params.size()>0) {
		int v = ccvpInt(params,0);
		CCSize sz = getContentSize();
		setContentSize(CCSizeMake(sz.width, v));
	}
	CCSize sz2 = getContentSize();
	return CCValue::intValue(sz2.height);
}
CCValue CCNode::CALLNAME(rotationX)(CCValueArray& params) {
	if(params.size()>0) {
		setRotationX(params[0].floatValue());
	}
	return CCValue::numberValue(getRotationX());
}
CCValue CCNode::CALLNAME(rotationY)(CCValueArray& params) {
	if(params.size()>0) {
		setRotationY(params[0].floatValue());
	}
	return CCValue::numberValue(getRotationY());
}
CCValue CCNode::CALLNAME(rotation)(CCValueArray& params) {
	if(params.size()>0) {
		setRotation(params[0].floatValue());
	}
	return CCValue::numberValue(getRotation());
}
CCValue CCNode::CALLNAME(ignoreAnchorPointForPosition)(CCValueArray& params) {
	if(params.size()>0) {
		ignoreAnchorPointForPosition(params[0].booleanValue());
	}
	return CCValue::booleanValue(isIgnoreAnchorPointForPosition());
}
CCValue CCNode::CALLNAME(visible)(CCValueArray& params) {
	if(params.size()>0) {
		setVisible(params[0].booleanValue());
	}
	return CCValue::booleanValue(isVisible());
}
CCValue CCNode::CALLNAME(scaleX)(CCValueArray& params) {
	if(params.size()>0) {
		setScaleX(params[0].floatValue());
	}
	return CCValue::numberValue(getScaleX());
}
CCValue CCNode::CALLNAME(scaleY)(CCValueArray& params) {
	if(params.size()>0) {
		setScaleY(params[0].floatValue());
	}	
	return CCValue::numberValue(getScaleY());
}
CCValue CCNode::CALLNAME(scale)(CCValueArray& params) {
	if(params.size()>0) {
		setScale(params[0].floatValue());
	}
	return CCValue::numberValue(getScale());
}
CCValue CCNode::CALLNAME(id)(CCValueArray& params) {
	if(params.size()>0) {
		setId(params[0].stringValue().c_str());
	}
	return CCValue::stringValue(getId());
}
CCValue CCNode::CALLNAME(tag)(CCValueArray& params) {
	if(params.size()>0) {
		setTag(params[0].intValue());
	}
	return CCValue::intValue(getTag());
}
CCValue CCNode::CALLNAME(positionX)(CCValueArray& params) {
	if(params.size()>0) {
		setPositionX(params[0].floatValue());
	}	
	return CCValue::numberValue(getPositionX());
}
CCValue CCNode::CALLNAME(positionY)(CCValueArray& params) {
	if(params.size()>0) {
		setPositionY(params[0].floatValue());
	}	
	return CCValue::numberValue(getPositionY());
}
CCValue CCNode::CALLNAME(position)(CCValueArray& params) {
	if(params.size()==1) {
		CCPoint pt = ccvpPoint(params, 0);
		setPosition(pt);
		return CCValue::nullValue();
	} else if(params.size()==2) {
		float x = params[0].floatValue();
		float y = params[1].floatValue();
		setPosition(x, y);
		return CCValue::nullValue();
	}
	float x = getPositionX();
	float y = getPositionY();
	return CCValueUtil::point(x, y);
}
CCValue CCNode::CALLNAME(skewX)(CCValueArray& params) {
	if(params.size()>0) {
		setSkewX(params[0].floatValue());
	}	
	return CCValue::numberValue(getSkewX());
}
CCValue CCNode::CALLNAME(skewY)(CCValueArray& params) {
	if(params.size()>0) {
		setSkewY(params[0].floatValue());
	}	
	return CCValue::numberValue(getSkewY());
}
CCValue CCNode::CALLNAME(zOrder)(CCValueArray& params) {
	if(params.size()>0) {
		setZOrder(params[0].intValue());
	}
	return CCValue::intValue(getZOrder());
}
CCValue CCNode::CALLNAME(onEvent)(CCValueArray& params) {
	std::string name = ccvpString(params, 0);
	std::string id = ccvpString(params,1);
	CCValue call = params.size()>2?params[2]:CCValue::nullValue();
	if(name.length()==0 || id.length()==0 || !call.canCall()) {
		throw new std::string("invalid params for onEvent");
	}
	onEvent(name.c_str(), id.c_str(), call);
	return CCValue::booleanValue(true);
}
CCValue CCNode::CALLNAME(removeEvent)(CCValueArray& params) {
	std::string name = ccvpString(params, 0);
	std::string id = ccvpString(params,1);
	return CCValue::booleanValue(removeEventHandler(name.c_str(), id.c_str()));
}
// end cc_call

// catyguan
// methods
void CCNode::bindMethod(const char* name, CCValue call)
{
	if(m_pMethods==NULL) {
		m_pMethods = new CCValueMap();
	}
	CCValueMapIterator it = m_pMethods->find(name);
	if(it!=m_pMethods->end()) {
		m_pMethods->erase(it);
	}
	(*m_pMethods)[name] = call;
	m_pMethods->at(name).retain();
}
void CCNode::removeMethod(const char* name)
{
	if(m_pMethods!=NULL) {
		CCValueMapIterator it = m_pMethods->find(name);
		if(it!=m_pMethods->end()) {
			m_pMethods->erase(it);
		}
	}
}
void CCNode::removeAllMethods()
{
	if(m_pMethods!=NULL) {
		m_pMethods->clear();
	}
}
// attribute
bool CCNode::hasAttribute(const char* name)
{
	if(m_pAttributes!=NULL) {
		return m_pAttributes->find(name)!=m_pAttributes->end();
	}
	return false;
}
CCValue CCNode::attribute(const char* name)
{
	if(m_pAttributes!=NULL) {
		CCValueMapIterator it = m_pAttributes->find(name);
		if(it!=m_pAttributes->end()) {
			return it->second;
		}
	}
	return CCValue::nullValue();
}
void CCNode::attribute(const char* name, CCValue v)
{	
	if(v.isNull()) {
		removeAttribute(name);
	} else {
		if(m_pAttributes==NULL)m_pAttributes = new CCValueMap();
		(*m_pAttributes)[name] = v;
		(*m_pAttributes)[name].retain();
	}
}
bool CCNode::removeAttribute(const char* name)
{
	if(m_pAttributes!=NULL) {
		CCValueMapIterator it = m_pAttributes->find(name);
		if(it!=m_pAttributes->end()) {
			m_pAttributes->erase(it);
			return true;
		}
	}
	return false;
}
void CCNode::clearAttributes()
{
	if(m_pAttributes!=NULL) {
		m_pAttributes->clear();
	}
}
// end attributes

// catyguan
// events
typedef std::list<CCNodeEventHandlerItem*>::const_iterator ehitor;

bool CCNode::hasEventHandler(const char* name)
{
	if(m_pEventHandlers!=NULL) {		
		for(ehitor it=m_pEventHandlers->begin();it!=m_pEventHandlers->end();it++) {
			CCNodeEventHandlerItem* h = (*it);
			if(h->type.compare(name)==0) {
				return true;
			}
		}
	}
	return false;
}
bool CCNode::raiseEvent(const char* name, CCNodeEvent* e)
{
	bool r = false;
	if(m_pEventHandlers!=NULL) {
		std::list<CCNodeEventHandlerItem*> tmp;
		CCValueArray ps;
		for(ehitor it=m_pEventHandlers->begin();m_pEventHandlers!=NULL && it!=m_pEventHandlers->end();) {
			CCNodeEventHandlerItem* h = (*it);
			it++;
			if(h->type.compare(name)==0) {
				if(h->handleObject!=NULL) {
					(h->handleObject->*h->handler)(this, name, e);
				} else {
					if(h->call.canCall()) {
						if(ps.size()==0) {
							ps.push_back(CCValue::objectValue(this));
							ps.push_back(CCValue::stringValue(name));
							if(e!=NULL) {
								ps.push_back(e->toValue());
							}
						}
						h->call.call(ps, false);
					}
				}
				r = true;
			}
		}
		ps.clear();
	}
	return r;
}
void CCNode::onEvent(const char* name,CCObject* obj,SEL_NodeEventHandler handler)
{
	onEvent(name,NULL, obj,handler,CCValue::nullValue());
}
void CCNode::onEvent(const char* name,const char* id, CCValue call)
{
	onEvent(name,id,NULL,NULL,call);
}
void CCNode::onEvent(const char* name,const char* id, CCObject* obj,SEL_NodeEventHandler handler,CCValue call)
{
	if(m_pEventHandlers==NULL)m_pEventHandlers = new std::list<CCNodeEventHandlerItem*>();
	CCNodeEventHandlerItem* h = new CCNodeEventHandlerItem();
	h->type = name;
	if(id!=NULL) {
		h->id = id;
	}
	if(obj!=NULL) {
		h->handleObject = obj;
		if(h->handleObject!=this) {
			CC_SAFE_RETAIN(obj);
		}
		h->handler = handler;
	} else {
		h->handleObject = NULL;
		h->handler = NULL;
	}
	h->call = call;
	h->call.retain();
	m_pEventHandlers->push_back(h);
}
bool CCNode::removeEventHandler(const char* name,const char* id, CCObject* obj)
{
	bool r = false;
	if(m_pEventHandlers!=NULL) {
		for(ehitor it=m_pEventHandlers->begin();it!=m_pEventHandlers->end();) {		
			ehitor cur = it;
			it++;
			CCNodeEventHandlerItem* h = (*cur);
			bool nm = false;
			bool om = false;
			bool idm = false;
			if(name!=NULL) {
				nm = h->type.compare(name)==0;
			} else {
				nm = true;
			}
			if(id!=NULL) {
				idm = h->id.compare(id)==0;
			} else {
				idm = true;
			}
			if(obj!=NULL) {
				om = h->handleObject==obj;
			} else {
				om = true;
			}
			if(om && nm && idm) {
				m_pEventHandlers->erase(cur);
				if(h->handleObject!=this) {
					CC_SAFE_RELEASE(h->handleObject);
				}
				h->call.cleanup();
				delete h;
				r = true;
			}		
		}
	}
	return r;
}
bool CCNode::removeEventHandler(const char* name,CCObject* obj)
{
	return removeEventHandler(name,NULL,obj);
}
bool CCNode::removeEventHandler(const char* name,const char* id)
{
	return removeEventHandler(name,id,NULL);
}
void CCNode::clearEventHandlers()
{
	if(m_pEventHandlers!=NULL) {
		for(ehitor it=m_pEventHandlers->begin();it!=m_pEventHandlers->end();it++) {
			CCNodeEventHandlerItem* h = (*it);
			if(h->handleObject!=this) {
				CC_SAFE_RELEASE(h->handleObject);
			}
			h->call.cleanup();
			delete h;
		}
		m_pEventHandlers->clear();
	}	
}
// end events

// CCNodeRGBA
CCNodeRGBA::CCNodeRGBA()
: _displayedOpacity(255)
, _realOpacity(255)
, _displayedColor(ccWHITE)
, _realColor(ccWHITE)
, _cascadeColorEnabled(false)
, _cascadeOpacityEnabled(false)
{}

CCNodeRGBA::~CCNodeRGBA() {}

bool CCNodeRGBA::init()
{
    if (CCNode::init())
    {
        _displayedOpacity = _realOpacity = 255;
        _displayedColor = _realColor = ccWHITE;
        _cascadeOpacityEnabled = _cascadeColorEnabled = false;
        return true;
    }
    return false;
}

GLubyte CCNodeRGBA::getOpacity(void)
{
	return _realOpacity;
}

GLubyte CCNodeRGBA::getDisplayedOpacity(void)
{
	return _displayedOpacity;
}

void CCNodeRGBA::setOpacity(GLubyte opacity)
{
    _displayedOpacity = _realOpacity = opacity;
    
	if (_cascadeOpacityEnabled)
    {
		GLubyte parentOpacity = 255;
        CCRGBAProtocol* pParent = dynamic_cast<CCRGBAProtocol*>(m_pParent);
        if (pParent && pParent->isCascadeOpacityEnabled())
        {
            parentOpacity = pParent->getDisplayedOpacity();
        }
        this->updateDisplayedOpacity(parentOpacity);
	}
}

void CCNodeRGBA::updateDisplayedOpacity(GLubyte parentOpacity)
{
	_displayedOpacity = _realOpacity * parentOpacity/255.0;
	
    if (_cascadeOpacityEnabled)
    {
        CCObject* pObj;
        CCARRAY_FOREACH(m_pChildren, pObj)
        {
            CCRGBAProtocol* item = dynamic_cast<CCRGBAProtocol*>(pObj);
            if (item)
            {
                item->updateDisplayedOpacity(_displayedOpacity);
            }
        }
    }
}

bool CCNodeRGBA::isCascadeOpacityEnabled(void)
{
    return _cascadeOpacityEnabled;
}

void CCNodeRGBA::setCascadeOpacityEnabled(bool cascadeOpacityEnabled)
{
    _cascadeOpacityEnabled = cascadeOpacityEnabled;
}

const ccColor3B& CCNodeRGBA::getColor(void)
{
	return _realColor;
}

const ccColor3B& CCNodeRGBA::getDisplayedColor()
{
	return _displayedColor;
}

void CCNodeRGBA::setColor(const ccColor3B& color)
{
	_displayedColor = _realColor = color;
	
	if (_cascadeColorEnabled)
    {
		ccColor3B parentColor = ccWHITE;
        CCRGBAProtocol *parent = dynamic_cast<CCRGBAProtocol*>(m_pParent);
		if (parent && parent->isCascadeColorEnabled())
        {
            parentColor = parent->getDisplayedColor(); 
        }
        
        updateDisplayedColor(parentColor);
	}
}

void CCNodeRGBA::updateDisplayedColor(const ccColor3B& parentColor)
{
	_displayedColor.r = _realColor.r * parentColor.r/255.0;
	_displayedColor.g = _realColor.g * parentColor.g/255.0;
	_displayedColor.b = _realColor.b * parentColor.b/255.0;
    
    if (_cascadeColorEnabled)
    {
        CCObject *obj = NULL;
        CCARRAY_FOREACH(m_pChildren, obj)
        {
            CCRGBAProtocol *item = dynamic_cast<CCRGBAProtocol*>(obj);
            if (item)
            {
                item->updateDisplayedColor(_displayedColor);
            }
        }
    }
}

bool CCNodeRGBA::isCascadeColorEnabled(void)
{
    return _cascadeColorEnabled;
}

void CCNodeRGBA::setCascadeColorEnabled(bool cascadeColorEnabled)
{
    _cascadeColorEnabled = cascadeColorEnabled;
}

// catyguan
// cc_call
CC_BEGIN_CALLS(CCNodeRGBA, CCNode)
	CC_DEFINE_CALL(CCNodeRGBA, opacity)
	CC_DEFINE_CALL(CCNodeRGBA, color)
	CC_DEFINE_CALL(CCNodeRGBA, cascadeColorEnabled)
	CC_DEFINE_CALL(CCNodeRGBA, opacityModifyRGB)
CC_END_CALLS(CCNodeRGBA, CCNode)

CCValue CCNodeRGBA::CALLNAME(opacity)(CCValueArray& params) {	 
	if(params.size()>0) {
		int v = params[0].intValue();
		setOpacity(v & 0xFF);
	}
	return CCValue::intValue(getOpacity());
}
CCValue CCNodeRGBA::CALLNAME(color)(CCValueArray& params) {	 
	if(params.size()>0) {
		ccColor3B v = CCValueUtil::color3b(params[0]);
		setColor(v);
		return CCValue::nullValue();
	} else {
		ccColor3B v = getColor();
		return CCValueUtil::color3b(v);
	}
}
CCValue CCNodeRGBA::CALLNAME(cascadeColorEnabled)(CCValueArray& params) {	 
	if(params.size()>0) {
		bool v = params[0].booleanValue();
		setCascadeColorEnabled(v);
	}
	return CCValue::booleanValue(isCascadeColorEnabled());
}
CCValue CCNodeRGBA::CALLNAME(opacityModifyRGB)(CCValueArray& params) {	 
	if(params.size()>0) {
		bool v = params[0].booleanValue();
		setOpacityModifyRGB(v);
	}
	return CCValue::booleanValue(isOpacityModifyRGB());
}
// end cc_call

NS_CC_END
