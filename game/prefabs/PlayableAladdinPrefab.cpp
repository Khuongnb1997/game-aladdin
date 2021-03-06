#include "PlayableAladdinPrefab.h"
#include "../Define.h"
#include "../scripts/DirectionController.h"
#include "../scripts/PlayableAladdinController.h"
#include "../scripts/CollisionTracker.h"

USING_NAMESPACE_ALA;

ALA_CLASS_SOURCE_1(PlayableAladdinPrefab, ala::Prefab)

void PlayableAladdinPrefab::doInstantiate( ala::GameObject* object, std::istringstream& argsStream ) const {
  // args
  const auto startX = nextFloat( argsStream );
  const auto startY = nextFloat( argsStream );

  // constants
  const auto gameManager = GameManager::get();
  const auto myAppData = static_cast<MyAppData*>(gameManager->getResource( "My App Data" ));

  const auto input = Input::get();
  const auto density = 5.0f;
  const auto runVelocity = 140.0f;
  const auto inAirVelocity = 140.0f;
  const auto climbVelocity = 60.0f;
  const auto holdBarMoveVelocity = 80.0f;
  const auto jumpImpulse = 3000000.0f;
  const auto jumpOnCamelImpulse = 3000000.0f;
  const auto jumpOnSpringImpulse = 3500000.0f;
  const auto stopAcceleration = 4.0f;
  const auto throwImpulse = Vec2( 20000.0f, 1000.0f );
  const auto swordOffset1 = Vec2( 45, 10 );
  const auto swordSize1 = Size( 40, 45 );
  const auto swordOffset2 = Vec2( 55, -2.5f );
  const auto swordSize2 = Size( 40, 20 );
  const auto swordOffset3 = Vec2( 30, 10 );
  const auto swordSize3 = Size( 35, 40 );
  const auto swordOffset4 = Vec2( 42, 8 );
  const auto swordSize4 = Size( 40, 55 );
  const auto swordOffset5 = Vec2( -35, 20 );
  const auto swordSize5 = Size( 25, 40 );
  const auto swordOffset6 = Vec2( 35, 25 );
  const auto swordSize6 = Size( 35, 45 );
  const auto swordOffset7 = Vec2( 0, 5 );
  const auto swordSize7 = Size( 65, 80 );

  // components
  const auto spriteRenderer = new SpriteRenderer( object, "aladdin.png" );

  const auto animator = new Animator( object, "idle_1", "aladdin.anm" );

  const auto highSwordAudio = new AudioSource( object, "High Sword.wav" );

  const auto throwAudio = new AudioSource( object, "Object Throw.wav" );

  const auto pushAudio = new AudioSource( object, "Aladdin Push.wav" );

  const auto hurtAudio = new AudioSource( object, "Aladdin Hurt.wav" );

  const auto lowSwordAudio = new AudioSource( object, "Low Sword.wav" );

  const auto body = new Rigidbody( object, PhysicsMaterial( density ), ALA_BODY_TYPE_DYNAMIC, 1.0f );

  const auto collider = new Collider( object, false, Vec2( 0, 0 ), Size( 40, 50 ), 1, 0, "Body" );
  collider->setTag( ALADDIN_TAG );
  collider->ignoreTag( ALADDIN_TAG );
  collider->ignoreTag( ENEMY_TAG );

  const auto swordCollider = new Collider( object, true, Vec2(), Size( 0, 0 ), 0, 0, "Sword" );
  swordCollider->setTag( SWORD_TAG );
  swordCollider->ignoreTag( ALADDIN_TAG );
  swordCollider->setActive( false );

  const auto stateManager = new StateManager( object, "initial" );

  const auto actionManager = new ActionManager( object );

  const auto direction = new DirectionController( object, true, 1 );
  direction->setApplyPhysics( false );
  direction->addReverseCase( [=] {
    return animator->getActionName().substr( 0, 8 ) == "hold_bar" ||
      animator->getActionName() == "climb_attack" || animator->getActionName() == "climb_throw";
  } );

  const auto controller = new PlayableAladdinController( object, "Controller" );

  // helpers
  const auto transform = object->getTransform();

  const auto timer1 = new Timer( object );

  const auto timer2 = new Timer( object );

  const auto timer3 = new Timer( object );

  const auto timer4 = new Timer( object );

  // collider renderers
  //  new ColliderRenderer( collider );
  //  new ColliderRenderer( swordCollider );

  // flags
  collider->setFlags( BELONGS_TO_ALADDIN | COLLIDE_FREE_OBJECT_FLAG );
  collider->ignoreIfNotHasAnyFlags( COLLIDE_ALADDIN_FLAG );
  collider->ignoreIfHasAnyFlags( BELONGS_TO_ALADDIN );
  swordCollider->setFlags( BELONGS_TO_ALADDIN | COLLIDE_ENEMY_FLAG | COLLIDE_FREE_OBJECT_FLAG );
  swordCollider->ignoreIfHasAnyFlags( BELONGS_TO_ALADDIN );

  // configurations
  object->setTag( ALADDIN_TAG );
  object->setLayer( "Main Character" );
  controller->setLives( 3 );
  controller->setApples( 10 );
  controller->setHealth( 9 );

  // states

  new State( stateManager, "initial",
             [=] {
               // animation effect
               {
                 if ( myAppData->getCurrentCheckpoint() == 0 ) {
                   transform->setPosition( startX, startY );
                 }
                 else {
                   std::stringstream checkpointNameBuilder;
                   checkpointNameBuilder << "Checkpoint " << myAppData->getCurrentCheckpoint();

                   const auto checkpoint = gameManager->getObjectByName( checkpointNameBuilder.str() );
                   transform->setPosition( checkpoint->getTransform()->getPositionX(),
                                           checkpoint->getTransform()->getPositionY() );
                   animator->setAction( "revive" );
                 }
               }

               // sword
               {
                 swordCollider->setActive( false );
               }

               // move
               {
                 controller->setMovingVelocityX( 0 );
               }
             }, NULL,
             [=] {
               // animation effect
               {
                 if ( myAppData->getCurrentCheckpoint() != 0 ) {
                   transform->setPositionY( transform->getPositionY() + 20 );
                 }
               }
             } );

  new State( stateManager, "idle",
             [=] {
               // animation effect
               {
                 if ( stateManager->getPreviousStateName() == "fall" ) {
                   if ( animator->getActionName() == "jump_attack_fall" ||
                     animator->getActionName() == "jump_throw_apple_fall" ||
                     animator->getActionName() == "run_to_jump_fall" ) {
                     animator->setAction( "touched_ground" );
                   }
                   else {
                     animator->setAction( "fall_to_idle_1" );
                   }
                 }
                 else if ( stateManager->getPreviousStateName() == "jump_attack" ||
                   stateManager->getPreviousStateName() == "jump_throw" ) {
                   animator->setAction( "touched_ground" );
                 }
                 else if ( stateManager->getPreviousStateName() == "face_up" ) {
                   animator->setAction( "face_up_to_idle_1" );
                 }
                 else if ( stateManager->getPreviousStateName() == "sit" ) {
                   animator->setAction( "stand_up" );
                 }
                 else {
                   animator->setAction( "idle_1" );
                   timer1->start( 0.5f );
                 }
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 controller->setMovingVelocityX( 0 );
               }

               // hit
               {
                 controller->resetHit();
               }
             },
             [=]( float dt ) {
               // animation effect
               {
                 if ( !animator->isPlaying() && timer1->isDone() ) {
                   if ( animator->getActionName() == "idle_4_to_1" ||
                     animator->getActionName() == "fall_to_idle_1" ||
                     animator->getActionName() == "touched_ground" ||
                     animator->getActionName() == "face_up_to_idle_1" ||
                     animator->getActionName() == "stand_up" ) {
                     animator->setAction( "idle_1" );
                     timer1->start( 0.5f );
                   }
                   if ( animator->getActionName() == "idle_1" || animator->getActionName() == "idle_4_to_1" ) {
                     animator->setAction( "idle_1_to_2" );
                   }
                   else if ( animator->getActionName() == "idle_1_to_2" || animator->getActionName() == "idle_3_to_2"
                   ) {
                     const auto way = rand() % 5;
                     if ( way == 0 ) {
                       animator->setAction( "idle_2_to_4" );
                     }
                     else {
                       animator->setAction( "idle_2" );
                       timer1->start( 1.0f * (200 + rand() % 900) / 1000 );
                     }
                   }
                   else if ( animator->getActionName() == "idle_2" ) {
                     animator->setAction( "idle_2_to_3" );
                   }
                   else if ( animator->getActionName() == "idle_2_to_3" ) {
                     animator->setAction( "idle_3" );
                     timer1->start( 1.0f * (200 + rand() % 900) / 1000 );
                   }
                   else if ( animator->getActionName() == "idle_3" ) {
                     animator->setAction( "idle_3_to_2" );
                   }
                   else if ( animator->getActionName() == "idle_2_to_4" ||
                     animator->getActionName() == "idle_4_to_4_way_1" ||
                     animator->getActionName() == "idle_4_to_4_way_2" ) {
                     const auto way = rand() % 9;
                     if ( way < 4 ) {
                       animator->setAction( "idle_4_to_4_way_1" );
                     }
                     else if ( way < 8 ) {
                       animator->setAction( "idle_4_to_4_way_2" );
                     }
                     else {
                       animator->setAction( "idle_4_to_1" );
                     }
                   }
                 }
               }

               // change direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                   animator->setAction( "idle_1" );
                   timer1->start( 0.5f );
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                   animator->setAction( "idle_1" );
                   timer1->start( 0.5f );
                 }
               }
             }, NULL );

  new State( stateManager, "attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "attack_1" );
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset1 );
                 swordCollider->setSize( swordSize1 );
                 swordCollider->setActive( false );
                 timer3->start( 0.2f );
               }
             },
             [=]( float dt ) {
               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 0.2f );

                     // audio
                     {
                       highSwordAudio->play();
                     }
                   }
                   else {
                     swordCollider->setActive( false );
                   }
                 }
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }
             } );

  new State( stateManager, "throw",
             [=] {
               // animation effect
               {
                 animator->setAction( "attack_3" );
               }

               // throw
               {
                 timer1->start( 0.1f );
               }
             },
             [=]( float ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // throw 
               {
                 if ( timer1->isDone() ) {
                   if ( direction->isRight() ) {
                     controller->throwApple( 'R', 0.0f, 0.0f, throwImpulse.getX(), throwImpulse.getY() );
                   }
                   else {
                     controller->throwApple( 'L', 0.0f, 0.0f, throwImpulse.getX(), throwImpulse.getY() );
                   }

                   timer1->start( 5.0f );

                   // audio
                   {
                     throwAudio->play();
                   }
                 }
               }
             }, NULL );

  new State( stateManager, "face_up",
             [=] {
               // animation effect 
               {
                 if ( stateManager->getPreviousStateName() == "face_up_attack" ||
                   stateManager->getPreviousStateName() == "face_up_throw" ) {
                   animator->setAction( "face_is_up" );
                 }
                 else {
                   animator->setAction( "face_up" );
                 }
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                   if ( !animator->isPlaying() ) {
                     animator->setAction( "face_is_up" );
                   }
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                   if ( !animator->isPlaying() ) {
                     animator->setAction( "face_is_up" );
                   }
                 }
               }
             }, NULL );

  new State( stateManager, "face_up_attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "attack_2" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset5 );
                 swordCollider->setSize( swordSize5 );
                 swordCollider->setActive( false );
                 timer3->start( 0.2f );
               }
             },
             [=]( float dt ) {
               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 0.25f );

                     // audio 
                     {
                       highSwordAudio->play();
                     }
                   }
                   else {
                     swordCollider->setActive( false );
                     if ( swordCollider->getOffset() == swordOffset5 ) {
                       swordCollider->setOffset( swordOffset6 );
                       swordCollider->setSize( swordSize6 );
                       timer3->start( 0.15f );
                     }
                     else {
                       timer3->start( 5 );
                     }
                   }
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }
             } );

  new State( stateManager, "face_up_throw",
             [=] {
               // animation effect
               {
                 animator->setAction( "attack_3" );
               }

               // throw
               {
                 timer1->start( 0.1f );
               }
             },
             [=]( float ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // throw
               {
                 if ( timer1->isDone() ) {
                   if ( direction->isRight() ) {
                     controller->throwApple( 'R', 0.0f, 0.0f, throwImpulse.getX(), throwImpulse.getY() );
                   }
                   else {
                     controller->throwApple( 'L', 0.0f, 0.0f, throwImpulse.getX(), throwImpulse.getY() );
                   }

                   timer1->start( 5.0f );

                   // audio
                   {
                     throwAudio->play();
                   }
                 }
               }
             }, NULL );

  new State( stateManager, "sit",
             [=] {
               // animation effect 
               {
                 if ( stateManager->getPreviousStateName() == "sit_attack" ) {
                   animator->setAction( "sit_attack_1_to_sit" );
                 }
                 else if ( stateManager->getPreviousStateName() == "sit_throw" ) {
                   animator->setAction( "sit_attack_2_to_sit" );
                 }
                 else {
                   animator->setAction( "sit_down" );
                 }
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                   if ( !animator->isPlaying() ) {
                     animator->setAction( "sit" );
                   }
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                   if ( !animator->isPlaying() ) {
                     animator->setAction( "sit" );
                   }
                 }
               }
             }, NULL );

  new State( stateManager, "sit_attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "sit_attack_1" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset2 );
                 swordCollider->setSize( swordSize2 );
                 swordCollider->setActive( false );
                 timer3->start( 0.2f );
               }
             },
             [=]( float dt ) {
               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 0.2f );

                     // audio
                     {
                       lowSwordAudio->play();
                     }
                   }
                   else {
                     swordCollider->setActive( false );
                   }
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }
             } );

  new State( stateManager, "sit_throw",
             [=] {
               // animation effect
               {
                 animator->setAction( "sit_attack_2" );
               }

               // throw
               {
                 timer1->start( 0.1f );
               }
             },
             [=]( float ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // throw
               {
                 if ( timer1->isDone() ) {
                   if ( direction->isRight() ) {
                     controller->throwApple( 'R', 0, -collider->getSize().getHeight() / 4,
                                             throwImpulse.getX(), throwImpulse.getY() );
                   }
                   else {
                     controller->throwApple( 'L', 0, -collider->getSize().getHeight() / 4,
                                             throwImpulse.getX(), throwImpulse.getY() );
                   }

                   timer1->start( 5.0f );

                   // audio
                   {
                     throwAudio->play();
                   }
                 }
               }
             }, NULL );

  new State( stateManager, "run",
             [=] {
               // animation effect
               {
                 animator->setAction( "start_run" );
                 timer1->start( 2 );
               }
             },
             [=]( float dt ) {
               // animation effect
               {
                 if ( !animator->isPlaying() && animator->getActionName() == "start_run" ) {
                   animator->setAction( "run" );
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 controller->setMovingVelocityX( runVelocity );
               }
             },NULL );

  new State( stateManager, "stop",
             [=] {
               // animation effect
               {
                 animator->setAction( "stop" );
               }
             },
             [=]( float dt ) {
               // move
               {
                 float newVelocity = 0;
                 if ( body->getVelocity() < 0 ) {
                   newVelocity = body->getVelocity().getX() + stopAcceleration;
                   if ( newVelocity > 0 ) newVelocity = 0;
                 }
                 else if ( body->getVelocity() > 0 ) {
                   newVelocity = body->getVelocity().getX() - stopAcceleration;
                   if ( newVelocity < 0 ) newVelocity = 0;
                 }

                 controller->setMovingVelocityX( newVelocity );
               }
             }, NULL );

  new State( stateManager, "run_attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "run_attack" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset3 );
                 swordCollider->setSize( swordSize3 );
                 swordCollider->setActive( false );
                 timer3->start( 0.2f );
               }
             },
             [=]( float dt ) {
               // move
               {
                 controller->setMovingVelocityX( runVelocity );
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 0.2f );

                     // audio
                     {
                       highSwordAudio->play();
                     }

                   }
                   else {
                     swordCollider->setActive( false );
                   }
                 }
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }
             } );

  new State( stateManager, "jump",
             [=] {
               // animation effect
               {
                 if ( stateManager->getPreviousStateName() == "run" ) {
                   animator->setAction( "run_to_jump" );
                 }
                 else if ( stateManager->getPreviousStateName() == "climb" ) {
                   animator->setAction( "climb_to_jump" );
                 }
                 else if ( controller->isJumpOnSpring() ) {
                   animator->setAction( "jump_rotate" );
                 }
                 else {
                   animator->setAction( "jump" );
                 }
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 if ( controller->isJumpOnCamel() ) {
                   body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                   body->addImpulse( Vec2( 0, jumpOnCamelImpulse ) );
                 }
                 else if ( controller->isJumpOnSpring() ) {
                   body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                   body->addImpulse( Vec2( 0, jumpOnSpringImpulse ) );
                 }
                 else {
                   body->addImpulse( Vec2( 0, jumpImpulse ) );
                 }
               }

               // climb collision
               {
                 controller->resetHoldingRope();
               }

               // standable collsion
               {
                 controller->resetCollidedWithStandable();
               }

               // camel collision
               {
                 controller->resetJumpOnCamel();
               }

               // spring collision
               {
                 controller->resetJumpOnSpring();
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 if ( (direction->isLeft() && input->getKey( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKey( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( inAirVelocity );
                 }
                 if ( (direction->isLeft() && input->getKeyUp( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKeyUp( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( 0 );
                 }
               }
             }, NULL );

  new State( stateManager, "fall",
             [=] {
               // animation effect
               {
                 if ( stateManager->getPreviousStateName() == "jump_attack" ) {
                   animator->setAction( "jump_attack_fall" );
                 }
                 else if ( stateManager->getPreviousStateName() == "jump_throw" ) {
                   animator->setAction( "jump_throw_apple_fall" );
                 }
                 else {
                   if ( animator->getActionName() == "run_to_jump" ) {
                     animator->setAction( "run_to_jump_fall" );
                   }
                   else if ( animator->getActionName() == "climb_to_jump" ) {
                     animator->setAction( "climb_to_jump_fall" );
                   }
                   else if ( animator->getActionName() == "jump_rotate" ) {
                     animator->setAction( "jump_rotate_fall" );
                   }
                   else {
                     animator->setAction( "fall" );
                   }
                 }
               }

               // standable collision
               {
                 controller->resetCollidedWithStandable();
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 if ( (direction->isLeft() && input->getKey( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKey( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( inAirVelocity );
                 }

                 if ( (direction->isLeft() && input->getKeyUp( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKeyUp( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( 0 );
                 }
               }
             }, NULL );

  new State( stateManager, "jump_attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "jump_attack" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset4 );
                 swordCollider->setSize( swordSize4 );
                 swordCollider->setActive( false );
                 timer3->start( 0.4f );
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 if ( (direction->isLeft() && input->getKey( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKey( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( inAirVelocity );
                 }

                 if ( (direction->isLeft() && input->getKeyUp( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKeyUp( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( 0 );
                 }
               }

               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 0.2f );

                     // audio
                     {
                       highSwordAudio->play();
                     }
                   }
                   else {
                     swordCollider->setActive( false );
                     timer3->start( 5 );
                   }
                 }
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }
             } );

  new State( stateManager, "jump_throw",
             [=] {
               // animation effect
               {
                 animator->setAction( "jump_throw_apple" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // throw
               {
                 timer2->start( 0.15f );
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move
               {
                 if ( (direction->isLeft() && input->getKey( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKey( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( inAirVelocity );
                 }

                 if ( (direction->isLeft() && input->getKeyUp( ALA_KEY_LEFT_ARROW ))
                   || (direction->isRight() && input->getKeyUp( ALA_KEY_RIGHT_ARROW )) ) {
                   controller->setMovingVelocityX( 0 );
                 }
               }

               // throw
               {
                 if ( timer2->isDone() ) {
                   if ( direction->isRight() ) {
                     controller->throwApple( 'R', 0.0f, -(collider->getSize().getHeight() / 4), throwImpulse.getX(),
                                             throwImpulse.getY() );
                   }
                   else {
                     controller->throwApple( 'L', 0.0f, -(collider->getSize().getHeight() / 4), throwImpulse.getX(),
                                             throwImpulse.getY() );
                   }
                   timer2->start( 2.0f );

                   // audio
                   {
                     throwAudio->play();
                   }
                 }
               }
             }, NULL );

  new State( stateManager, "push",
             [=] {
               // animation effect
               {
                 animator->setAction( "start_push" );
               }

               // audio
               {
                 pushAudio->play();
                 timer4->start( 1.0f );
               }
             },
             [=]( float ) {
               // animation effect
               {
                 if ( !animator->isPlaying() ) {
                   animator->setAction( "pushing" );
                 }
               }

               // audio
               {
                 if ( timer4->isDone() ) {
                   pushAudio->play();
                   timer4->start( 1.0f );
                 }
               }

               // move
               {
                 if ( (input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isRight())
                   || (input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isLeft()) ) {
                   controller->setMovingVelocityX( runVelocity );
                 }
                 else {
                   controller->setMovingVelocityX( 0 );
                 }
               }
             }, NULL );

  new State( stateManager, "climb",
             [=] {
               // animation effect
               {
                 animator->setAction( "climb" );
                 animator->pause();
               }

               // move
               {
                 body->setGravityScale( 0 );
               }
             },
             [=]( float dt ) {
               auto dir = 'C';
               if ( input->getKey( ALA_KEY_UP_ARROW ) ) {
                 dir = 'U';
               }
               else if ( input->getKey( ALA_KEY_DOWN_ARROW ) ) {
                 dir = 'D';
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // animation effect
               {
                 if ( dir == 'C' ) {
                   animator->pause();
                 }
                 else {
                   if ( dir == 'U' ) {
                     if ( !controller->hasReachedTopOfRope() ) {
                       animator->setReverse( false );
                       animator->play();
                     }
                     else {
                       animator->pause();
                     }
                   }
                   else if ( dir == 'D' ) {
                     animator->setReverse( true );
                     animator->play();
                   }
                 }
               }

               // move
               {
                 if ( dir == 'U' ) {
                   if ( !controller->hasReachedTopOfRope() ) {
                     body->setVelocity( Vec2( body->getVelocity().getX(), climbVelocity ) );
                   }
                   else {
                     body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                   }
                 }
                 else if ( dir == 'D' ) {
                   body->setVelocity( Vec2( body->getVelocity().getX(), -climbVelocity ) );
                 }
                 else {
                   body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 }

                 controller->setMovingVelocityX( 0 );
               }

               // position
               {
                 transform->setPositionX( controller->getHodingRope()->getTransform()->getPositionX() );
               }
             },
             [=] {
               // animation effect 
               {
                 animator->setReverse( false );
               }

               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
               }
             } );

  new State( stateManager, "climb_attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "climb_attack" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset7 );
                 swordCollider->setSize( swordSize7 );
                 swordCollider->setActive( false );
                 timer3->start( 0.2f );
               }

               // move
               {
                 body->setGravityScale( 0 );
               }
             },
             [=]( float dt ) {
               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 5 );

                     // audio
                     {
                       highSwordAudio->play();
                     }
                   }
                   else {
                     swordCollider->setActive( false );
                   }
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // move 
               {
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }

               // position
               {
                 transform->setPositionX( controller->getHodingRope()->getTransform()->getPositionX() );
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }

               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
               }
             } );

  new State( stateManager, "climb_throw",
             [=] {
               // animation effect
               {
                 animator->setAction( "climb_throw" );
               }

               // throw
               {
                 timer1->start( 0.2f );
               }

               // move
               {
                 body->setGravityScale( 0 );
               }
             },
             [=]( float ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // throw
               {
                 if ( timer1->isDone() ) {
                   if ( direction->isRight() ) {
                     controller->throwApple( 'R', 0, -collider->getSize().getHeight() * 0.55f,
                                             throwImpulse.getX(), throwImpulse.getY() );
                   }
                   else {
                     controller->throwApple( 'L', 0, -collider->getSize().getHeight() * 0.55f,
                                             throwImpulse.getX(), throwImpulse.getY() );
                   }

                   timer1->start( 5.0f );

                   // audio
                   {
                     throwAudio->play();
                   }
                 }
               }

               // move 
               {
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }

               // position
               {
                 transform->setPositionX( controller->getHodingRope()->getTransform()->getPositionX() );
               }
             },
             [=] {
               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
               }
             } );

  new State( stateManager, "hold_bar_idle",
             [=] {
               // animation effect
               {
                 animator->setAction( "hold_bar_idle_0" );
                 timer1->start( (rand() % 10 + 1) / 10.0f );
               }

               // move
               {
                 body->setGravityScale( 0 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             },
             [=]( float dt ) {
               // animation effect
               {
                 if ( !animator->isPlaying() && timer1->isDone() ) {
                   if ( animator->getActionName() == "hold_bar_idle_0" ) {
                     animator->setAction( "hold_bar_idle_0_1" );
                   }
                   else if ( animator->getActionName() == "hold_bar_idle_0_1" ) {
                     animator->setAction( "hold_bar_idle_1" );
                     timer1->start( (rand() % 10 + 1) / 10.0f );
                   }
                   else if ( animator->getActionName() == "hold_bar_idle_1" ) {
                     animator->setAction( "hold_bar_idle_1_0" );
                   }
                   else if ( animator->getActionName() == "hold_bar_idle_1_0" ) {
                     animator->setAction( "hold_bar_idle_0" );
                     timer1->start( (rand() % 10 + 1) / 10.0f );
                   }
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // position
               {
                 transform->setPositionY( controller->getHoldingBar()->getTransform()->getPositionY() );
               }
             },
             [=] {
               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             } );

  new State( stateManager, "hold_bar_move",
             [=] {
               // animation effect
               {
                 animator->setAction( "hold_bar_move" );
               }

               // move
               {
                 body->setGravityScale( 0 );
               }
             },
             [=]( float dt ) {
               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // position
               {
                 transform->setPositionY( controller->getHoldingBar()->getTransform()->getPositionY() );
               }

               // move
               {
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( holdBarMoveVelocity );
               }
             },
             [=] {
               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             } );

  new State( stateManager, "hold_bar_attack",
             [=] {
               // animation effect
               {
                 animator->setAction( "climb_attack" );
               }

               // direction
               {
                 if ( input->getKey( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKey( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // sword
               {
                 swordCollider->setOffset( swordOffset7 );
                 swordCollider->setSize( swordSize7 );
                 swordCollider->setActive( false );
                 timer3->start( 0.2f );
               }

               // move
               {
                 body->setGravityScale( 0 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             },
             [=]( float dt ) {
               // sword
               {
                 if ( timer3->isDone() ) {
                   if ( !swordCollider->isActive() ) {
                     swordCollider->setActive( true );
                     timer3->start( 5 );

                     // audio
                     {
                       highSwordAudio->play();
                     }
                   }
                   else {
                     swordCollider->setActive( false );
                   }
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // position
               {
                 transform->setPositionY( controller->getHoldingBar()->getTransform()->getPositionY() );
               }
             },
             [=] {
               // sword
               {
                 swordCollider->setActive( false );
               }

               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             } );

  new State( stateManager, "hold_bar_throw",
             [=] {
               // animation effect
               {
                 animator->setAction( "climb_throw" );
               }

               // throw
               {
                 timer1->start( 0.2f );
               }

               // move
               {
                 body->setGravityScale( 0 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             },
             [=]( float dt ) {
               // throw
               {
                 if ( timer1->isDone() ) {
                   if ( direction->isRight() ) {
                     controller->throwApple( 'R', 0, -collider->getSize().getHeight() * 0.55f,
                                             throwImpulse.getX(), throwImpulse.getY() );
                   }
                   else {
                     controller->throwApple( 'L', 0, -collider->getSize().getHeight() * 0.55f,
                                             throwImpulse.getX(), throwImpulse.getY() );
                   }

                   timer1->start( 5.0f );

                   // audio
                   {
                     throwAudio->play();
                   }
                 }
               }

               // direction
               {
                 if ( input->getKeyDown( ALA_KEY_LEFT_ARROW ) && direction->isRight() ) {
                   direction->setLeft();
                 }
                 else if ( input->getKeyDown( ALA_KEY_RIGHT_ARROW ) && direction->isLeft() ) {
                   direction->setRight();
                 }
               }

               // position
               {
                 transform->setPositionY( controller->getHoldingBar()->getTransform()->getPositionY() );
               }
             },
             [=] {
               // move
               {
                 body->setGravityScale( 1 );
                 body->setVelocity( Vec2( body->getVelocity().getX(), 0 ) );
                 controller->setMovingVelocityX( 0 );
               }
             } );

  new State( stateManager, "hit",
             [=] {
               // animation effect 
               {
                 animator->setAction( "hit" );
               }

               // audio
               {
                 hurtAudio->play();
               }

               // move
               {
                 //                 body->setVelocity( Vec2( 0, body->getVelocity().getY() ) );
                 controller->setMovingVelocityX( 0 );
               }

               // reset hit
               {
                 controller->resetHit();
               }
             }, NULL, NULL );

  new StateTransition( stateManager, "initial", "idle", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "idle", "attack", [=] {
    return input->getKeyDown( ALA_KEY_S );
  } );

  new StateTransition( stateManager, "attack", "idle", [=] {
    return (!animator->isPlaying())
      || (direction->isRight() && input->getKey( ALA_KEY_LEFT_ARROW ))
      || (direction->isLeft() && input->getKey( ALA_KEY_RIGHT_ARROW ));;
  } );

  new StateTransition( stateManager, "idle", "throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && controller->getApples() > 0;
  } );

  new StateTransition( stateManager, "throw", "idle", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "idle", "sit", [=] {
    return input->getKey( ALA_KEY_DOWN_ARROW );
  } );

  new StateTransition( stateManager, "attack", "sit", [=] {
    return input->getKey( ALA_KEY_DOWN_ARROW );
  } );

  new StateTransition( stateManager, "sit", "idle", [=] {
    return !input->getKey( ALA_KEY_DOWN_ARROW ) && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "sit", "sit_attack", [=] {
    return input->getKeyDown( ALA_KEY_S ) && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "sit_attack", "sit", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "sit", "sit_throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && !animator->isPlaying() && controller->getApples() > 0;
  } );

  new StateTransition( stateManager, "sit_throw", "sit", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "idle", "face_up", [=] {
    return input->getKey( ALA_KEY_UP_ARROW );
  } );

  new StateTransition( stateManager, "attack", "face_up", [=] {
    return input->getKey( ALA_KEY_UP_ARROW );
  } );

  new StateTransition( stateManager, "face_up", "idle", [=] {
    return !input->getKey( ALA_KEY_UP_ARROW ) && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "face_up", "face_up_attack", [=] {
    return input->getKeyDown( ALA_KEY_S ) && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "face_up_attack", "face_up", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "face_up", "face_up_throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && !animator->isPlaying() && controller->getApples() > 0;
  } );

  new StateTransition( stateManager, "face_up_throw", "face_up", [=] {
    return input->getKey( ALA_KEY_UP_ARROW ) && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "idle", "run", [=] {
    return ((direction->isRight() && input->getKey( ALA_KEY_RIGHT_ARROW ))
      || (direction->isLeft() && input->getKey( ALA_KEY_LEFT_ARROW )));
  } );

  new StateTransition( stateManager, "run", "run_attack", [=] {
    return input->getKeyDown( ALA_KEY_S );
  } );

  new StateTransition( stateManager, "run_attack", "run", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "run", "stop", [=] {
    return !input->getKey( ALA_KEY_RIGHT_ARROW ) && !input->getKey( ALA_KEY_LEFT_ARROW ) && timer1->isDone();
  } );

  new StateTransition( stateManager, "run", "idle", [=] {
    return (direction->isRight() && !input->getKey( ALA_KEY_RIGHT_ARROW ))
      || (direction->isLeft() && !input->getKey( ALA_KEY_LEFT_ARROW ));
  } );

  new StateTransition( stateManager, "stop", "idle", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "idle", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "attack", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "throw", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "sit", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "sit_attack", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "sit_throw", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "face_up", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "face_up_attack", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "face_up_throw", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "run", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "jump", "jump_attack", [=] {
    return input->getKeyDown( ALA_KEY_S );
  } );

  new StateTransition( stateManager, "jump", "jump_throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && controller->getApples() > 0;
  } );

  new StateTransition( stateManager, "jump", "fall", [=] {
    return body->getVelocity().getY() < -10;
  } );

  new StateTransition( stateManager, "idle", "fall", [=] {
    return body->getVelocity().getY() < -10;
  } );

  new StateTransition( stateManager, "stop", "fall", [=] {
    return body->getVelocity().getY() < -10;
  } );

  // TODO: fix for falling
  //  new StateTransition( stateManager, "run", "fall", [=] {
  //    return body->getVelocity().getY() < -10;
  //  } );

  new StateTransition( stateManager, "jump_attack", "fall", [=] {
    return body->getVelocity().getY() < -10 && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "jump_throw", "fall", [=] {
    return body->getVelocity().getY() < -10 && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "fall", "jump_attack", [=] {
    return input->getKeyDown( ALA_KEY_S );
  } );

  new StateTransition( stateManager, "fall", "jump_throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && controller->getApples() > 0;
  } );

  new StateTransition( stateManager, "fall", "idle", [=] {
    return controller->isCollidedWithStandable();
  } );

  new StateTransition( stateManager, "jump", "idle", [=] {
    return controller->isCollidedWithStandable();
  } );

  new StateTransition( stateManager, "jump_attack", "idle", [=] {
    return controller->isCollidedWithStandable();
  } );

  new StateTransition( stateManager, "jump_throw", "idle", [=] {
    return controller->isCollidedWithStandable();
  } );

  new StateTransition( stateManager, "idle", "hit", [=] {
    return controller->isHit();
  } );

  new StateTransition( stateManager, "hit", "idle", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "run", "push", [=] {
    return controller->isPushingWall();
  } );

  new StateTransition( stateManager, "push", "idle", [=] {
    return !controller->isPushingWall();
  } );

  new StateTransition( stateManager, "climb", "fall", [=] {
    return !controller->isHoldingRope();
  } );

  new StateTransition( stateManager, "climb", "jump", [=] {
    return input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "climb", "climb_attack", [=] {
    return input->getKeyDown( ALA_KEY_S ) && !animator->isPlaying();
  } );

  new StateTransition( stateManager, "climb_attack", "climb", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "climb", "climb_throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && !animator->isPlaying() && controller->getApples() > 0;
  } );

  new StateTransition( stateManager, "climb_throw", "climb", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "hold_bar_idle", "hold_bar_move", [=] {
    return ((direction->isRight() && input->getKey( ALA_KEY_RIGHT_ARROW ))
      || (direction->isLeft() && input->getKey( ALA_KEY_LEFT_ARROW )));
  } );

  new StateTransition( stateManager, "hold_bar_move", "hold_bar_idle", [=] {
    return (direction->isRight() && !input->getKey( ALA_KEY_RIGHT_ARROW ))
      || (direction->isLeft() && !input->getKey( ALA_KEY_LEFT_ARROW ));
  } );

  new StateTransition( stateManager, "hold_bar_idle", "hold_bar_attack", [=] {
    return input->getKeyDown( ALA_KEY_S );
  } );

  new StateTransition( stateManager, "hold_bar_attack", "hold_bar_idle", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "hold_bar_idle", "hold_bar_throw", [=] {
    return input->getKeyDown( ALA_KEY_A ) && controller->getApples() > 0;;
  } );

  new StateTransition( stateManager, "hold_bar_throw", "hold_bar_idle", [=] {
    return !animator->isPlaying();
  } );

  new StateTransition( stateManager, "hold_bar_idle", "fall", [=] {
    return !controller->isHoldingBar() || input->getKeyDown( ALA_KEY_D );
  } );

  new StateTransition( stateManager, "hold_bar_move", "fall", [=] {
    return !controller->isHoldingBar();
  } );
}
