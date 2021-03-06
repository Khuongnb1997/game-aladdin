#include "ThrowableKnifePrefab.h"
#include "../Define.h"
#include "../scripts/DirectionController.h"
#include "../scripts/CollisionTracker.h"

USING_NAMESPACE_ALA;

ALA_CLASS_SOURCE_1(ThrowableKnifePrefab, ala::PrefabV2)

void ThrowableKnifePrefab::doInstantiate( ala::GameObject* object, std::istringstream& argsStream ) const {
  // args
  const auto dir = nextChar( argsStream );
  const auto impulseX = nextFloat( argsStream );
  const auto impulseY = nextFloat( argsStream );

  // constants
  const auto gameManager = GameManager::get();
  const auto input = Input::get();
  const auto audioPlayerPrefab = gameManager->getPrefabV2( "Audio Player" );

  const auto density = 1.0f;

  // components
  const auto spriteRenderer = new SpriteRenderer( object, "guards.png" );

  const auto animator = new Animator( object, "throwable_knife", "guards.anm" );

  const auto body = new Rigidbody( object, PhysicsMaterial( density ), ALA_BODY_TYPE_DYNAMIC, 1.0f );

  const auto collider = new Collider( object, true, Vec2( 0, 0 ), Size( 10, 10 ), 0.49f );
  collider->setTag( KNIFE_TAG );
  collider->ignoreTag( KNIFE_TAG );
  collider->ignoreTag( CHARCOAL_BURNER_TAG );
  collider->ignoreTag( ENEMY_TAG );

  const auto stateManager = new StateManager( object, "initial" );

  const auto direction = new DirectionController( object );
  if ( dir == 'L' ) direction->setLeft();
  else if ( dir == 'R' ) direction->setRight();

  const auto collisionTracker = new CollisionTracker( object );

  // helpers
  const auto transform = object->getTransform();

  // collider renderers
  //  new ColliderRenderer( collider );

  // flags
  collider->setFlags( COLLIDE_ALADDIN_FLAG | COLLIDE_FREE_OBJECT_FLAG );
  collider->ignoreIfNotHasAnyFlags( COLLIDE_FREE_OBJECT_FLAG );

  // configurations
  object->setLayer( "Foreground" );
  object->setTag( ENEMY_TAG );

  // states
  new State( stateManager, "initial",
             [=] {
               // move
               {
                 body->addImpulse( Vec2( impulseX, impulseY ) );
               }
             },
             [=]( float dt ) {
               if ( collisionTracker->collided() ) {
                 // audio
                 {
                   audioPlayerPrefab->instantiateWithArgs( "Sword Ching.wav" );
                 }

                 // release
                 object->release();
               }
             }, NULL );
}
