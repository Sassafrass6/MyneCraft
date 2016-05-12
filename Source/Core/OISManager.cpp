#include <OISInputManager.h>

#include "MultiPlatformHelper.h"
#include "OISManager.h"
 
OISManager *OISManager::mOISManager;
 
OISManager::OISManager( void ) :
    cameraMan( 0 ),
    mMouse( 0 ),
    mKeyboard( 0 ),
    mInputSystem( 0 ) {
}
 
OISManager::~OISManager( void ) {
    if( mInputSystem ) {
        if( mMouse ) {
            mInputSystem->destroyInputObject( mMouse );
            mMouse = 0;
        }
 
        if( mKeyboard ) {
            mInputSystem->destroyInputObject( mKeyboard );
            mKeyboard = 0;
        }

        // If you use OIS1.0RC1 or above, uncomment this line
        // and comment the line below it
        mInputSystem->destroyInputSystem( mInputSystem );
        //mInputSystem->destroyInputSystem();
        mInputSystem = 0;
 
        // Clear Listeners
        mKeyListeners.clear();
        mMouseListeners.clear();
    }
}
 
void OISManager::initialise( Ogre::RenderWindow *renderWindow ) {
    if( !mInputSystem ) {

        // Setup basic variables
        OIS::ParamList paramList;    
        size_t windowHnd = 0;
        std::ostringstream windowHndStr;
 
        // Get window handle
        renderWindow->getCustomAttribute( "WINDOW", &windowHnd );
 
        // Fill parameter list
        windowHndStr << (unsigned int) windowHnd;
        paramList.insert( std::make_pair( std::string( "WINDOW" ), windowHndStr.str() ) );
 
        // Create inputsystem
        mInputSystem = OIS::InputManager::createInputSystem( paramList );
    
        if (mInputSystem->getNumberOfDevices(OIS::OISKeyboard) > 0) {
            mKeyboard = static_cast<OIS::Keyboard*>( mInputSystem->createInputObject( OIS::OISKeyboard, true ) );
            mKeyboard->setEventCallback( this );
        }
 
        if (mInputSystem->getNumberOfDevices(OIS::OISMouse) > 0) {
            mMouse = static_cast<OIS::Mouse*>( mInputSystem->createInputObject( OIS::OISMouse, true ) );
            mMouse->setEventCallback( this );   
 
            // Get window size
            unsigned int width, height, depth;
            int left, top;
            renderWindow->getMetrics( width, height, depth, left, top );
 
            // Set mouse region
            this->setWindowExtents( width, height );
        }

    }
}

void OISManager::setupCameraMan( Ogre::Camera* camMan ) {
    cameraMan = camMan;
}
 
void OISManager::capture( void ) {
    // Need to capture / update each device every frame
    if( mMouse ) {
        mMouse->capture();
    }
 
    if( mKeyboard ) {
        mKeyboard->capture();
    }
}
 
void OISManager::addKeyListener( OIS::KeyListener *keyListener, const std::string& instanceName ) {
    if( mKeyboard ) {
        // Check for duplicate items
        itKeyListener = mKeyListeners.find( instanceName );
        if( itKeyListener == mKeyListeners.end() ) {
            mKeyListeners[ instanceName ] = keyListener;
        }
        else {
            // Duplicate Item
        }
    }
}
 
void OISManager::addMouseListener( OIS::MouseListener *mouseListener, const std::string& instanceName ) {
    if( mMouse ) {
        // Check for duplicate items
        itMouseListener = mMouseListeners.find( instanceName );
        if( itMouseListener == mMouseListeners.end() ) {
            mMouseListeners[ instanceName ] = mouseListener;
        }
        else {
            // Duplicate Item
        }
    }
}
 
 
void OISManager::removeKeyListener( const std::string& instanceName ) {
    // Check if item exists
    itKeyListener = mKeyListeners.find( instanceName );
    if( itKeyListener != mKeyListeners.end() ) {
        mKeyListeners.erase( itKeyListener );
    }
    else {
        // Doesn't Exist
    }
}
 
void OISManager::removeMouseListener( const std::string& instanceName ) {
    // Check if item exists
    itMouseListener = mMouseListeners.find( instanceName );
    if( itMouseListener != mMouseListeners.end() ) {
        mMouseListeners.erase( itMouseListener );
    }
    else {
        // Doesn't Exist
    }
}

 
void OISManager::removeKeyListener( OIS::KeyListener *keyListener ) {
    itKeyListener    = mKeyListeners.begin();
    itKeyListenerEnd = mKeyListeners.end();
    for(; itKeyListener != itKeyListenerEnd; ++itKeyListener ) {
        if( itKeyListener->second == keyListener ) {
            mKeyListeners.erase( itKeyListener );
            break;
        }
    }
}
 
void OISManager::removeMouseListener( OIS::MouseListener *mouseListener ) {
    itMouseListener    = mMouseListeners.begin();
    itMouseListenerEnd = mMouseListeners.end();
    for(; itMouseListener != itMouseListenerEnd; ++itMouseListener ) {
        if( itMouseListener->second == mouseListener ) {
            mMouseListeners.erase( itMouseListener );
            break;
        }
    }
}

void OISManager::removeAllListeners( void ) {
    mKeyListeners.clear();
    mMouseListeners.clear();
}
 
void OISManager::removeAllKeyListeners( void ) {
    mKeyListeners.clear();
}
 
void OISManager::removeAllMouseListeners( void ) {
    mMouseListeners.clear();
}

void OISManager::setWindowExtents( int width, int height ) {
    // Set mouse region (if window resizes, we should alter this to reflect as well)
    const OIS::MouseState &mouseState = mMouse->getMouseState();
    mouseState.width  = width;
    mouseState.height = height;
}
 
OIS::Mouse* OISManager::getMouse( void ) {
    return mMouse;
}
 
OIS::Keyboard* OISManager::getKeyboard( void ) {
    return mKeyboard;
}

 
bool OISManager::keyPressed( const OIS::KeyEvent &e ) {
    mKeyPressed = e.key;

#if defined __linux__ || defined _DEBUG
    CEGUI::GUIContext& cxt = CEGUI::System::getSingleton().getDefaultGUIContext();
    cxt.injectKeyDown((CEGUI::Key::Scan)e.key);
    cxt.injectChar((CEGUI::Key::Scan)e.text);
#endif

    return true;
}

bool OISManager::isKeyDown(OIS::KeyCode key) {
	return mKeyboard->isKeyDown(key);
}

OIS::KeyCode OISManager::lastKeyPressed() {
    OIS::KeyCode ret = mKeyPressed;
    mKeyPressed = OIS::KC_UNASSIGNED;
    return ret;
}

bool OISManager::keyReleased( const OIS::KeyEvent &e ) {
	mKeyPressed = OIS::KC_UNASSIGNED;
    return true;
}
 
bool OISManager::mouseMoved( const OIS::MouseEvent &e ) {
    static Ogre::Degree curPitch(0);
    static Ogre::Degree maxPitch(75);
    static Ogre::Degree minPitch(-80);

    if(cameraMan) {
        cameraMan->yaw(Ogre::Degree(-e.state.X.rel * 0.15f));

        Ogre::Degree nextPitch(-e.state.Y.rel * 0.15f);
        Ogre::Degree tPitch = curPitch + nextPitch;
        if ( tPitch > minPitch && tPitch < maxPitch ) {
            curPitch = curPitch + nextPitch;
            cameraMan->pitch(nextPitch);
        }
    }

    // From -width/2 to +width/2
    mouseXAxis = (e.state.X.abs) - e.state.width/2;
    mouseYAxis = (e.state.Y.abs) - e.state.height/2;
	mouseWheel = e.state.Z.rel / 120.0f;
#if defined __linux__ || defined _DEBUG
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.getDefaultGUIContext().injectMousePosition(e.state.X.abs, e.state.Y.abs);
#endif

    return true;
}

void OISManager::resetWheel() {
	mouseWheel = 0;
}
 
bool OISManager::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
#if defined __linux__ || defined _DEBUG
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(convertButton(id));
#endif

    mouseClicked  = true;
    return true;
}
 
bool OISManager::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
#if defined __linux__ || defined _DEBUG
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(convertButton(id));
#endif

    mouseClicked  = false;
    return true;
}
 
OISManager* OISManager::getSingletonPtr( void ) {
    if( !mOISManager ) {
        mOISManager = new OISManager();
    }
 
    return mOISManager;
}

int OISManager::getMouseXAxis() {
    return mouseXAxis;
}

int OISManager::getMouseYAxis() {
    return mouseYAxis;
}

int OISManager::getMouseWheel() {
	return mouseWheel;
}

OIS::KeyCode OISManager::getKeyPressed(){
    return mKeyPressed;
}

CEGUI::MouseButton OISManager::convertButton(OIS::MouseButtonID buttonID) {
    switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;
 
    case OIS::MB_Right:
        return CEGUI::RightButton;
 
    case OIS::MB_Middle:
        return CEGUI::MiddleButton;
 
    default:
        return CEGUI::LeftButton;
    }
}