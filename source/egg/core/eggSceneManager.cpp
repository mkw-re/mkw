#include <egg/core/eggSceneManager.hpp>
#include <egg/core/eggSystem.hpp>
#include <egg/eggInternal.hpp>

extern "C" {
// PAL: 0x8023aef8..0x8023af18
UNKNOWN_FUNCTION(SceneManager_reinitCurrentScene);
// PAL: 0x8023af18..0x8023af84
UNKNOWN_FUNCTION(SceneManager_reinitCurrentSceneAfterFadeOut);
// PAL: 0x8023af84..0x8023afe0
UNKNOWN_FUNCTION(unk_8023af84);
// PAL: 0x8023afe0..0x8023afe8
UNKNOWN_FUNCTION(SceneManager_changeSiblingScene);
// PAL: 0x8023afe8..0x8023b064
UNKNOWN_FUNCTION(SceneManager_changeSiblingSceneAfterFadeOut);
// PAL: 0x8023b064..0x8023b0e4
UNKNOWN_FUNCTION(SceneManager_changeSiblingScene2);
// PAL: 0x8023b0e4..0x8023b248
UNKNOWN_FUNCTION(createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene);
// PAL: 0x8023b2ac..0x8023b344
UNKNOWN_FUNCTION(destroyCurrentSceneNoIncoming__Q23EGG12SceneManagerFb);
// PAL: 0x8023b344..0x8023b3f0
UNKNOWN_FUNCTION(destroyToSelectSceneID__Q23EGG12SceneManagerFi);
// PAL: 0x8023b3f0..0x8023b568
UNKNOWN_FUNCTION(destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene);
// PAL: 0x8023b568..0x8023b588
UNKNOWN_FUNCTION(incomingCurrentScene__Q23EGG12SceneManagerFv);
// PAL: 0x8023b588..0x8023b5a8
UNKNOWN_FUNCTION(calcCurrentScene__Q23EGG12SceneManagerFv);
// PAL: 0x8023b5a8..0x8023b800
UNKNOWN_FUNCTION(calcCurrentFader__Q23EGG12SceneManagerFv);
// PAL: 0x8023b800..0x8023b890
UNKNOWN_FUNCTION(drawCurrentScene__Q23EGG12SceneManagerFv);
// PAL: 0x8023b890..0x8023b8ac
UNKNOWN_FUNCTION(drawCurrentFader__Q23EGG12SceneManagerFv);
// PAL: 0x8023b8ac..0x8023b8b0
UNKNOWN_FUNCTION(unk_8023b8ac);
// PAL: 0x8023b8b0..0x8023b910
UNKNOWN_FUNCTION(createDefaultFader__Q23EGG12SceneManagerFv);
// PAL: 0x8023b910..0x8023b92c
UNKNOWN_FUNCTION(setupNextSceneID__Q23EGG12SceneManagerFv);
// PAL: 0x8023b92c..0x8023b940
UNKNOWN_FUNCTION(outgoingParentScene__Q23EGG12SceneManagerFPQ23EGG5Scene);
// PAL: 0x8023b940..0x8023b980
UNKNOWN_FUNCTION(findParentScene__Q23EGG12SceneManagerFi);

extern UNKNOWN_FUNCTION(GXFlush);
extern UNKNOWN_FUNCTION(GXDraw);
extern UNKNOWN_FUNCTION(__ct__Q23EGG10ColorFaderFffffQ34nw4r2ut5ColorQ33EGG5Fader7EStatus);
extern UNKNOWN_FUNCTION(create__Q23EGG7ExpHeapFUlPQ23EGG4HeapUs);
extern UNKNOWN_FUNCTION(becomeCurrentHeap__Q23EGG4HeapFv);
extern UNKNOWN_FUNCTION(__nw__FUl);
}

extern u32 unk_80389008, unk_8038900c, unk_80389010;

namespace EGG {

u32 SceneManager::sHeapOptionFlg;
Heap* SceneManager::sHeapMem1_ForCreateScene;
Heap* SceneManager::sHeapMem2_ForCreateScene;
Heap* SceneManager::sHeapDebug_ForCreateScene;

SceneManager::SceneManager(SceneCreator* creator) {
  mSceneCreator = creator;
  mCurrentScene = nullptr;
  _1C = -1;
  _18 = -1;
  mNextSceneID = -1;
  mTransitionStatus = STATUS_IDLE;
  mCurrentFader = nullptr;
  bUseMem2 = true;
  createDefaultFader();
}

bool SceneManager::fadeIn() { return mCurrentFader->fadeIn(); }

void SceneManager::calc() {
  calcCurrentScene();
  calcCurrentFader();
}

void SceneManager::draw() {
  drawCurrentScene();
  drawCurrentFader();
}

void SceneManager::reinitCurrentScene() {
  if (mCurrentScene != nullptr) {
    mCurrentScene->reinit();
  }
}

bool SceneManager::reinitCurrentSceneAfterFadeOut() {
  bool returnValue = false;

  if (mTransitionStatus == STATUS_IDLE && mCurrentFader->fadeOut()) {
    mTransitionStatus = STATUS_REINITIALIZE;
    returnValue = true;
  }

  return returnValue;
}

#if 0
// Exit current scene to parent, then manuver to sibling scene ID
// Not a real symbol
void SceneManager::ChangeUncleScene(int ID) {
  while (this->mCurrentScene) {
    // destroy current scene then go to parent
    this->destroyCurrentSceneNoIncoming(true);
  }
  this->changeSiblingScene(ID);
}

void SceneManager::changeSiblingScene(int ID) {
  this->mNextSceneID = ID;
  changeSiblingScene();
}

bool SceneManager::changeSiblingSceneAfterFadeOut(int ID) {
  bool returnValue = false;
  if (this->mTransitionStatus == -1 && !this->mCurrentFader->fadeOut()) {
    this->mNextSceneID = ID;
    this->mTransitionStatus = STATUS_CHANGE_SIBLING_SCENE;
    returnValue = true;
  }

  return returnValue;
}

void SceneManager::changeSiblingScene() {
  Scene* curScene = this->mCurrentScene; // r4

  Scene* parentScene = curScene ? curScene->getParentScene() : NULL; // r31

  if (curScene) {
    this->destroyScene(curScene);
    this->mCurrentScene = NULL;

    s32 r30 = mNextSceneID;
    this->setupNextSceneID();
    this->createScene(r30, parentScene);
  }
}

void SceneManager::createScene(int ID, Scene* parentScene) {
  Heap* pParentHeap_Mem1;  // r26
  Heap* pParentHeap_Mem2;  // r25
  Heap* pParentHeap_Debug; // r24

  // if the scene is not NULL (I imagine only NULL for root?), we want to
  // build off that
  if (parentScene) {
    pParentHeap_Mem1 = parentScene->getHeap_Mem1();
    pParentHeap_Mem2 = parentScene->getHeap_Mem2();
    pParentHeap_Debug = parentScene->getHeap_Debug();
  }
  // otherwise, grab the system heaps
  else {
    BaseSystem* sys = EGG::ConfigurationData::sConfigData;
    pParentHeap_Mem1 = sys->mRootHeapMem1;
    pParentHeap_Mem2 = sys->mRootHeapMem2;
    pParentHeap_Debug = sys->mRootHeapDebug;
  }

  Heap* pNewHeap = bUseMem2 ? pParentHeap_Mem2 : pParentHeap_Mem1; // r28

  WiiSportsAssert(pParentHeap_Mem1 && pParentHeap_Mem2, "eggSceneManager.cpp",
                  267, "pParentHeap_Mem1 && pParentHeap_Mem2");

  // same val from "DAME DAME!"
  int r27 = pNewHeap->mFlag & 1;
  if (r27)
    pNewHeap->mFlag &= ~1;

  ExpHeap* r23 = ExpHeap::create(-1, pNewHeap, sHeapOptionFlg); // r23

  ExpHeap* pNewHeap_Debug = NULL; // r22
  ExpHeap* pNewHeap_Mem1;         // r25; overwrites pParentHeap_Mem2
  ExpHeap* pNewHeap_Mem2;         // r26; overwrites pParentHeap_Mem1

  if (pNewHeap == pParentHeap_Mem2) {
    pNewHeap_Mem1 = ExpHeap::create(-1, pParentHeap_Mem1, sHeapOptionFlg);
    pNewHeap_Mem2 = (EGG::ExpHeap*)r23;
  } else {
    pNewHeap_Mem2 = ExpHeap::create(-1, pParentHeap_Mem2, sHeapOptionFlg);
    pNewHeap_Mem1 = (EGG::ExpHeap*)r23;
  }
  // If we have a debug heap, we want to create a child debug heap
  // The system debug heap will in normal game logic be created if and only if
  // MEM2 size exceeds retail size. Since the root entry, would build off
  // this, we would not expect this code to run on a game running on a non
  // development unit
  if (pParentHeap_Debug)
    pNewHeap_Debug = ExpHeap::create(-1, pParentHeap_Debug, sHeapOptionFlg);

  // Set static heap pointers
  sHeapMem1_ForCreateScene = pNewHeap_Mem1;
  sHeapMem2_ForCreateScene = pNewHeap_Mem2;
  sHeapDebug_ForCreateScene = pNewHeap_Debug;

  WiiSportsAssert(pNewHeap && pNewHeap_Mem1 && pNewHeap_Mem2,
                  "eggSceneManager.cpp", 299,
                  "pNewHeap && pNewHeap_Mem1 && pNewHeap_Mem2");

  // if we unset that value above, we need to restore it
  if (r27)
    pNewHeap->mFlag |= 1;

  // Let's make the new heap the current heap
  r23->becomeCurrentHeap();

  // Create the scene through the scene creator.
  WiiSportsAssert(this->mSceneCreator, "eggSceneManager.cpp", 311,
                  "mSceneCreator");
  Scene* pNewScene = this->mSceneCreator->create(ID); // r3
  WiiSportsAssert(pNewScene, "eggSceneManager.cpp", 313, "pNewScene");

  // Inline setters below
  if (parentScene)
    parentScene->setChildScene(pNewScene);
  this->mCurrentScene = pNewScene;
  pNewScene->setSceneID(ID);
  pNewScene->setParentScene(parentScene);
  pNewScene->setSceneMgr(this);
  pNewScene->enter();
}
void SceneManager::createChildScene(int ID, Scene* pScene) {
  this->outgoingParentScene(pScene);
  this->mNextSceneID = ID;
  this->setupNextSceneID();
  this->createScene(ID, pScene);
}

void SceneManager::drawCurrentScene() {
  if (!mCurrentScene)
    return;

  mCurrentScene->draw();

  Display* pSystemDisplay = BaseSystem::sConfigData->getDisplay(); // r31

  // flag name likely wrong
  if (BaseSystem::sConfigData->getVideo()->mFlag &
      Video::VIDEO_FLAG_IS_NOT_BLACKED_OUT)
    if (!(pSystemDisplay->mScreenStateFlag &
          Display::SCREEN_STATE_FLAG_TOGGLE_BLACK_OUT))
      pSystemDisplay->mScreenStateFlag |=
          Display::SCREEN_STATE_FLAG_TOGGLE_BLACK_OUT;
}
void SceneManager::drawCurrentFader() {
  if (mCurrentFader)
    mCurrentFader->draw();
}

Scene* SceneManager::findParentScene(int ID) {
  bool found = false; // r5
  Scene* scene;       // r3

  for (scene = mCurrentScene->getParentScene(); scene != NULL;
       scene = scene->getParentScene()) {
    if (scene->getSceneID() == ID) {
      found = true;
      break;
    }
  }

  return found ? scene : NULL;
}
#endif
} // namespace EGG

// Symbol: unk_8023af84
// PAL: 0x8023af84..0x8023afe0
MARK_BINARY_BLOB(unk_8023af84, 0x8023af84, 0x8023afe0);
asm UNKNOWN_FUNCTION(unk_8023af84) {
  // clang-format off
  nofralloc;
  stwu r1, -0x10(r1);
  mflr r0;
  stw r0, 0x14(r1);
  stw r31, 0xc(r1);
  mr r31, r4;
  stw r30, 8(r1);
  mr r30, r3;
  b lbl_8023afb0;
lbl_8023afa4:
  mr r3, r30;
  li r4, 1;
  bl destroyCurrentSceneNoIncoming__Q23EGG12SceneManagerFb;
lbl_8023afb0:
  lwz r0, 0xc(r30);
  cmpwi r0, 0;
  bne lbl_8023afa4;
  mr r3, r30;
  mr r4, r31;
  bl SceneManager_changeSiblingScene;
  lwz r0, 0x14(r1);
  lwz r31, 0xc(r1);
  lwz r30, 8(r1);
  mtlr r0;
  addi r1, r1, 0x10;
  blr;
  // clang-format on
}

// Symbol: SceneManager_changeSiblingScene
// PAL: 0x8023afe0..0x8023afe8
MARK_BINARY_BLOB(SceneManager_changeSiblingScene, 0x8023afe0, 0x8023afe8);
asm UNKNOWN_FUNCTION(SceneManager_changeSiblingScene) {
  // clang-format off
  nofralloc;
  stw r4, 0x14(r3);
  b SceneManager_changeSiblingScene2;
  // clang-format on
}

// Symbol: SceneManager_changeSiblingSceneAfterFadeOut
// PAL: 0x8023afe8..0x8023b064
MARK_BINARY_BLOB(SceneManager_changeSiblingSceneAfterFadeOut, 0x8023afe8,
                 0x8023b064);
asm UNKNOWN_FUNCTION(SceneManager_changeSiblingSceneAfterFadeOut) {
  // clang-format off
  nofralloc;
  stwu r1, -0x20(r1);
  mflr r0;
  stw r0, 0x24(r1);
  stw r31, 0x1c(r1);
  li r31, 0;
  stw r30, 0x18(r1);
  mr r30, r4;
  stw r29, 0x14(r1);
  mr r29, r3;
  lwz r0, 0x20(r3);
  cmpwi r0, -1;
  bne lbl_8023b044;
  lwz r3, 0x24(r3);
  lwz r12, 0(r3);
  lwz r12, 0x14(r12);
  mtctr r12;
  bctrl;
  cmpwi r3, 0;
  beq lbl_8023b044;
  li r0, 1;
  stw r30, 0x14(r29);
  li r31, 1;
  stw r0, 0x20(r29);
lbl_8023b044:
  mr r3, r31;
  lwz r31, 0x1c(r1);
  lwz r30, 0x18(r1);
  lwz r29, 0x14(r1);
  lwz r0, 0x24(r1);
  mtlr r0;
  addi r1, r1, 0x20;
  blr;
  // clang-format on
}

// Symbol: SceneManager_changeSiblingScene2
// PAL: 0x8023b064..0x8023b0e4
MARK_BINARY_BLOB(SceneManager_changeSiblingScene2, 0x8023b064, 0x8023b0e4);
asm UNKNOWN_FUNCTION(SceneManager_changeSiblingScene2) {
  // clang-format off
  nofralloc;
  stwu r1, -0x20(r1);
  mflr r0;
  stw r0, 0x24(r1);
  stw r31, 0x1c(r1);
  li r31, 0;
  stw r30, 0x18(r1);
  stw r29, 0x14(r1);
  mr r29, r3;
  lwz r4, 0xc(r3);
  cmpwi r4, 0;
  beq lbl_8023b094;
  lwz r31, 0x20(r4);
lbl_8023b094:
  cmpwi r4, 0;
  beq lbl_8023b0ac;
  mr r3, r29;
  bl destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  li r0, 0;
  stw r0, 0xc(r29);
lbl_8023b0ac:
  lwz r30, 0x14(r29);
  mr r3, r29;
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
  mr r3, r29;
  mr r4, r30;
  mr r5, r31;
  bl createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene;
  lwz r0, 0x24(r1);
  lwz r31, 0x1c(r1);
  lwz r30, 0x18(r1);
  lwz r29, 0x14(r1);
  mtlr r0;
  addi r1, r1, 0x20;
  blr;
  // clang-format on
}

// Symbol: createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene
// PAL: 0x8023b0e4..0x8023b248
MARK_BINARY_BLOB(createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene, 0x8023b0e4, 0x8023b248);
asm UNKNOWN_FUNCTION(createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene) {
  // clang-format off
  nofralloc;
  stwu r1, -0x30(r1);
  mflr r0;
  cmpwi r5, 0;
  stw r0, 0x34(r1);
  stmw r22, 8(r1);
  mr r29, r3;
  mr r30, r4;
  mr r31, r5;
  beq lbl_8023b118;
  lwz r26, 0x14(r5);
  lwz r25, 0x18(r5);
  lwz r24, 0x1c(r5);
  b lbl_8023b128;
lbl_8023b118:
  lwz r4, -0x5ca0(r13);
  lwz r26, 0x18(r4);
  lwz r25, 0x1c(r4);
  lwz r24, 0x20(r4);
lbl_8023b128:
  lwz r0, 0x28(r3);
  cmpwi r0, 0;
  bne lbl_8023b13c;
  mr r28, r26;
  b lbl_8023b140;
lbl_8023b13c:
  mr r28, r25;
lbl_8023b140:
  lhz r0, 0x1c(r28);
  clrlwi. r27, r0, 0x1f;
  beq lbl_8023b158;
  lhz r0, 0x1c(r28);
  rlwinm r0, r0, 0, 0x10, 0x1e;
  sth r0, 0x1c(r28);
lbl_8023b158:
  lhz r5, -0x5d00(r13);
  mr r4, r28;
  li r3, -1;
  bl create__Q23EGG7ExpHeapFUlPQ23EGG4HeapUs;
  cmplw r28, r25;
  mr r23, r3;
  li r22, 0;
  bne lbl_8023b194;
  lhz r5, -0x5d00(r13);
  mr r4, r26;
  li r3, -1;
  bl create__Q23EGG7ExpHeapFUlPQ23EGG4HeapUs;
  mr r25, r3;
  mr r26, r23;
  b lbl_8023b1ac;
lbl_8023b194:
  lhz r5, -0x5d00(r13);
  mr r4, r25;
  li r3, -1;
  bl create__Q23EGG7ExpHeapFUlPQ23EGG4HeapUs;
  mr r26, r3;
  mr r25, r23;
lbl_8023b1ac:
  cmpwi r24, 0;
  beq lbl_8023b1c8;
  lhz r5, -0x5d00(r13);
  mr r4, r24;
  li r3, -1;
  bl create__Q23EGG7ExpHeapFUlPQ23EGG4HeapUs;
  mr r22, r3;
lbl_8023b1c8:
  cmpwi r27, 0;
  stw r25, -0x5cfc(r13);
  stw r26, -0x5cf8(r13);
  stw r22, -0x5cf4(r13);
  beq lbl_8023b1e8;
  lhz r0, 0x1c(r28);
  ori r0, r0, 1;
  sth r0, 0x1c(r28);
lbl_8023b1e8:
  mr r3, r23;
  bl becomeCurrentHeap__Q23EGG4HeapFv;
  lwz r3, 4(r29);
  mr r4, r30;
  lwz r12, 0(r3);
  lwz r12, 8(r12);
  mtctr r12;
  bctrl;
  cmpwi r31, 0;
  beq lbl_8023b214;
  stw r3, 0x24(r31);
lbl_8023b214:
  stw r3, 0xc(r29);
  stw r30, 0x28(r3);
  stw r31, 0x20(r3);
  stw r29, 0x2c(r3);
  lwz r12, 0(r3);
  lwz r12, 0x14(r12);
  mtctr r12;
  bctrl;
  lmw r22, 8(r1);
  lwz r0, 0x34(r1);
  mtlr r0;
  addi r1, r1, 0x30;
  blr;
  // clang-format on
}

namespace EGG {

void SceneManager::createChildScene(int ID, Scene* pScene) {
  outgoingParentScene(pScene);
  mNextSceneID = ID;
  setupNextSceneID();
  createScene(ID, pScene);
}

bool SceneManager::destroyCurrentSceneNoIncoming(bool destroyRootIfNoParent) {
    bool ret = 0;
    Scene * scene = mCurrentScene;
    if( scene ) {
        Scene * parent = scene->getParentScene();
        if( parent ) {
            ret = 1;
            Scene * child = parent->getChildScene();
            destroyScene( child );
            mNextSceneID = parent->getSceneID();
            setupNextSceneID();
        } else {
            if( destroyRootIfNoParent ) {
                destroyScene( scene );
                mNextSceneID = -1;
                setupNextSceneID();
            }
        }
    }
    return ret;
}

bool EGG::SceneManager::destroyToSelectSceneID(int ID) {
  bool ret = 0;
  Scene* parent = this->findParentScene(ID); // r31
  if (parent) {
    ret = 1;
    // destroyCurrentSceneNoIncoming should be inlined
    while (parent->getSceneID() != getUnk18()) {
      Scene* parent; // r30
      if (this->mCurrentScene &&
          (parent = this->mCurrentScene->getParentScene())) {
        this->destroyScene(parent->getChildScene());
        this->mNextSceneID = parent->getSceneID();
        this->setupNextSceneID();
      }
    }
    this->incomingCurrentScene();
  }
  return ret;
}

void SceneManager::destroyScene(Scene* pScene /* r30 */) {
  pScene->exit();
  GXFlush();
  GXDraw();

  // If we have a child scene, destroy that and so on
  if (pScene->getChildScene())
    this->destroyScene(pScene->getChildScene());
  GXFlush();
  GXDraw();

  Scene* parent = pScene->getParentScene(); // r31
  this->mSceneCreator->destroy(pScene->getSceneID());
  this->mCurrentScene = 0;

  // If we're not the root scene, delete ourselves from our parent
  // and make the current scene our parent
  if (parent) {
    parent->setChildScene(0);
    this->mCurrentScene = parent;
  }

  // If the scene has a debug heap, destroy it
  if (pScene->getHeap_Debug())
    pScene->getHeap_Debug()->destroy();

  Heap * sceneHeap = pScene->getHeap(); // r4
  if (pScene->getHeap_Mem1() == sceneHeap) {
    pScene->getHeap_Mem2()->destroy();
    pScene->getHeap_Mem1()->destroy();
  } else if (pScene->getHeap_Mem2() == sceneHeap) {
    pScene->getHeap_Mem1()->destroy();
    pScene->getHeap_Mem2()->destroy();
  }
    
  Heap* r31_second;
  // If we have a parent, use it's heap
  if (parent) {
    r31_second = parent->getHeap();
  }
  // If we're root, rederive it
  else {
    r31_second = !this->bUseMem2 ? BaseSystem::sSystem->mRootHeapMem1
                                 : BaseSystem::sSystem->mRootHeapMem2;
  }
  GXFlush();
  GXDraw();
  r31_second->becomeCurrentHeap();
  GXFlush();
  GXDraw();
}

void SceneManager::incomingCurrentScene() {
  if (this->mCurrentScene)
    this->mCurrentScene->incoming_childDestroy();
}

void SceneManager::calcCurrentScene() {
  if (this->mCurrentScene)
    this->mCurrentScene->calc();
}

} // namespace EGG

#if 0
namespace EGG {

void SceneManager::calcCurrentFader() {
  if (!mCurrentFader)
    return;
  if (!mCurrentFader->calc())
    return;
  switch (this->mTransitionStatus) {
  case 0: {
    int r29 = this->mNextSceneID;
    Scene* curScene;                       // r4
    while (curScene = this->mCurrentScene) // r4
    {
      if (!curScene)
        continue;
      Scene* r28 = curScene->getParentScene(); // r28
      if (r28) {
        this->destroyScene(r28->getChildScene());
        this->mNextSceneID = r28->getSceneID();
        this->setupNextSceneID();
      } else {
        this->destroyScene(curScene);
        this->mNextSceneID = -1;
        this->setupNextSceneID();
      }
    }
    this->mNextSceneID = r29;
    EGG::Scene* r30 = 0;
    if( curScene ) r30 = curScene->getParentScene();
    if( curScene ) {
      this->destroyScene(curScene);
      this->mCurrentScene = 0;
    }

    this->mNextSceneID = r29;
    this->setupNextSceneID();
    this->createScene(r29, r30);
    break;
  }
  case 1: {
    // Identical to changeSiblingScene
    Scene* r30 = 0;
    if( mCurrentScene ) r30 = mCurrentScene->getParentScene();
    if (mCurrentScene) {
      this->destroyScene(mCurrentScene);
      this->mCurrentScene = 0;
    }
    int r29 = this->mNextSceneID;
    this->setupNextSceneID();
    this->createScene(r29, r30);
    break;
  }
  case 2:
    this->outgoingParentScene(this->pParent);
    this->setupNextSceneID();
    this->createScene(this->get18(), this->pParent);
    break;
  case 4:
    // TODO
    if (Scene* s = this->findParentScene(this->mNextSceneID)) // r30
    {
      while (s->getSceneID() != this->get18()) {
        if (this->mCurrentScene == 0)
          continue;
        Scene* r28 = mCurrentScene->getParentScene();
        if (r28 == 0)
          continue;
        this->destroyScene(r28->getChildScene());
        this->mNextSceneID = r28->getSceneID();
        this->setupNextSceneID();
      }
      if (this->mCurrentScene == 0)
        break;
      this->mCurrentScene->incoming_childDestroy();
    }

    break;
  case 3:
    if (mCurrentScene)
      mCurrentScene->reinit();
    break;
  }
  this->mTransitionStatus = -1;
}

}
#else

// Symbol: calcCurrentFader__Q23EGG12SceneManagerFv
// PAL: 0x8023b5a8..0x8023b800
MARK_BINARY_BLOB(calcCurrentFader__Q23EGG12SceneManagerFv, 0x8023b5a8,
                 0x8023b800);
asm UNKNOWN_FUNCTION(calcCurrentFader__Q23EGG12SceneManagerFv) {
  // clang-format off
  nofralloc;
  stwu r1, -0x20(r1);
  mflr r0;
  stw r0, 0x24(r1);
  stw r31, 0x1c(r1);
  mr r31, r3;
  stw r30, 0x18(r1);
  stw r29, 0x14(r1);
  stw r28, 0x10(r1);
  lwz r0, 0x24(r3);
  cmpwi r0, 0;
  beq lbl_8023b7e0;
  mr r3, r0;
  lwz r12, 0(r3);
  lwz r12, 0x18(r12);
  mtctr r12;
  bctrl;
  cmpwi r3, 0;
  beq lbl_8023b7e0;
  lwz r0, 0x20(r31);
  cmpwi r0, 0;
  beq lbl_8023b620;
  cmpwi r0, 1;
  beq lbl_8023b6c8;
  cmpwi r0, 2;
  beq lbl_8023b714;
  cmpwi r0, 4;
  beq lbl_8023b73c;
  cmpwi r0, 3;
  beq lbl_8023b7bc;
  b lbl_8023b7d8;
lbl_8023b620:
  lwz r29, 0x14(r31);
  li r30, -1;
  b lbl_8023b674;
lbl_8023b62c:
  cmpwi r4, 0;
  beq lbl_8023b674;
  lwz r28, 0x20(r4);
  cmpwi r28, 0;
  beq lbl_8023b660;
  lwz r4, 0x24(r28);
  mr r3, r31;
  bl destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  lwz r0, 0x28(r28);
  mr r3, r31;
  stw r0, 0x14(r31);
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
  b lbl_8023b674;
lbl_8023b660:
  mr r3, r31;
  bl destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  stw r30, 0x14(r31);
  mr r3, r31;
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
lbl_8023b674:
  lwz r4, 0xc(r31);
  cmpwi r4, 0;
  bne lbl_8023b62c;
  stw r29, 0x14(r31);
  li r30, 0;
  beq lbl_8023b690;
  lwz r30, 0x20(r4);
lbl_8023b690:
  cmpwi r4, 0;
  beq lbl_8023b6a8;
  mr r3, r31;
  bl destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  li r0, 0;
  stw r0, 0xc(r31);
lbl_8023b6a8:
  lwz r29, 0x14(r31);
  mr r3, r31;
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
  mr r3, r31;
  mr r4, r29;
  mr r5, r30;
  bl createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene;
  b lbl_8023b7d8;
lbl_8023b6c8:
  lwz r4, 0xc(r31);
  li r30, 0;
  cmpwi r4, 0;
  beq lbl_8023b6dc;
  lwz r30, 0x20(r4);
lbl_8023b6dc:
  cmpwi r4, 0;
  beq lbl_8023b6f4;
  mr r3, r31;
  bl destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  li r0, 0;
  stw r0, 0xc(r31);
lbl_8023b6f4:
  lwz r29, 0x14(r31);
  mr r3, r31;
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
  mr r3, r31;
  mr r4, r29;
  mr r5, r30;
  bl createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene;
  b lbl_8023b7d8;
lbl_8023b714:
  lwz r4, 0x10(r31);
  mr r3, r31;
  bl outgoingParentScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  mr r3, r31;
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
  lwz r4, 0x18(r31);
  mr r3, r31;
  lwz r5, 0x10(r31);
  bl createScene__Q23EGG12SceneManagerFiPQ23EGG5Scene;
  b lbl_8023b7d8;
lbl_8023b73c:
  lwz r4, 0x14(r31);
  mr r3, r31;
  bl findParentScene__Q23EGG12SceneManagerFi;
  cmpwi r3, 0;
  mr r30, r3;
  beq lbl_8023b7d8;
  b lbl_8023b78c;
lbl_8023b758:
  lwz r3, 0xc(r31);
  cmpwi r3, 0;
  beq lbl_8023b78c;
  lwz r28, 0x20(r3);
  cmpwi r28, 0;
  beq lbl_8023b78c;
  lwz r4, 0x24(r28);
  mr r3, r31;
  bl destroyScene__Q23EGG12SceneManagerFPQ23EGG5Scene;
  lwz r0, 0x28(r28);
  mr r3, r31;
  stw r0, 0x14(r31);
  bl setupNextSceneID__Q23EGG12SceneManagerFv;
lbl_8023b78c:
  lwz r3, 0x28(r30);
  lwz r0, 0x18(r31);
  cmpw r3, r0;
  bne lbl_8023b758;
  lwz r3, 0xc(r31);
  cmpwi r3, 0;
  beq lbl_8023b7d8;
  lwz r12, 0(r3);
  lwz r12, 0x20(r12);
  mtctr r12;
  bctrl;
  b lbl_8023b7d8;
lbl_8023b7bc:
  lwz r3, 0xc(r31);
  cmpwi r3, 0;
  beq lbl_8023b7d8;
  lwz r12, 0(r3);
  lwz r12, 0x1c(r12);
  mtctr r12;
  bctrl;
lbl_8023b7d8:
  li r0, -1;
  stw r0, 0x20(r31);
lbl_8023b7e0:
  lwz r0, 0x24(r1);
  lwz r31, 0x1c(r1);
  lwz r30, 0x18(r1);
  lwz r29, 0x14(r1);
  lwz r28, 0x10(r1);
  mtlr r0;
  addi r1, r1, 0x20;
  blr;
  // clang-format on
}
#endif

// Symbol: drawCurrentScene__Q23EGG12SceneManagerFv
// PAL: 0x8023b800..0x8023b890
MARK_BINARY_BLOB(drawCurrentScene__Q23EGG12SceneManagerFv, 0x8023b800,
                 0x8023b890);
asm UNKNOWN_FUNCTION(drawCurrentScene__Q23EGG12SceneManagerFv) {
  // clang-format off
  nofralloc;
  stwu r1, -0x10(r1);
  mflr r0;
  stw r0, 0x14(r1);
  stw r31, 0xc(r1);
  lwz r3, 0xc(r3);
  cmpwi r3, 0;
  beq lbl_8023b87c;
  lwz r12, 0(r3);
  lwz r12, 0x10(r12);
  mtctr r12;
  bctrl;
  lwz r3, -0x5ca0(r13);
  lwz r12, 0(r3);
  lwz r12, 0x10(r12);
  mtctr r12;
  bctrl;
  mr r31, r3;
  lwz r3, -0x5ca0(r13);
  lwz r12, 0(r3);
  lwz r12, 8(r12);
  mtctr r12;
  bctrl;
  lbz r0, 4(r3);
  clrlwi. r0, r0, 0x1f;
  beq lbl_8023b87c;
  lbz r0, 9(r31);
  clrlwi. r0, r0, 0x1f;
  bne lbl_8023b87c;
  lbz r0, 9(r31);
  ori r0, r0, 1;
  stb r0, 9(r31);
lbl_8023b87c:
  lwz r0, 0x14(r1);
  lwz r31, 0xc(r1);
  mtlr r0;
  addi r1, r1, 0x10;
  blr;
  // clang-format on
}

// Symbol: drawCurrentFader__Q23EGG12SceneManagerFv
// PAL: 0x8023b890..0x8023b8ac
MARK_BINARY_BLOB(drawCurrentFader__Q23EGG12SceneManagerFv, 0x8023b890,
                 0x8023b8ac);
asm UNKNOWN_FUNCTION(drawCurrentFader__Q23EGG12SceneManagerFv) {
  // clang-format off
  nofralloc;
  lwz r3, 0x24(r3);
  cmpwi r3, 0;
  beqlr;
  lwz r12, 0(r3);
  lwz r12, 0x1c(r12);
  mtctr r12;
  bctr;
  // clang-format on
}

// Symbol: unk_8023b8ac
// PAL: 0x8023b8ac..0x8023b8b0
MARK_BINARY_BLOB(unk_8023b8ac, 0x8023b8ac, 0x8023b8b0);
asm UNKNOWN_FUNCTION(unk_8023b8ac) {
  // clang-format off
  nofralloc;
  blr;
  // clang-format on
}

#if 0
namespace EGG {
void SceneManager::createDefaultFader() {
  mCurrentFader = new ColorFader(0.0f, 0.0f, 640.0f, 480.0f, 0 /*color*/,
                                 Fader::ESTATUS_OPAQUE);
}
}
#else
// Symbol: createDefaultFader__Q23EGG12SceneManagerFv
// PAL: 0x8023b8b0..0x8023b910
MARK_BINARY_BLOB(createDefaultFader__Q23EGG12SceneManagerFv, 0x8023b8b0,
                 0x8023b910);
asm UNKNOWN_FUNCTION(createDefaultFader__Q23EGG12SceneManagerFv) {
  // clang-format off
  nofralloc;
  stwu r1, -0x20(r1);
  mflr r0;
  stw r0, 0x24(r1);
  stw r31, 0x1c(r1);
  mr r31, r3;
  li r3, 0x24;
  bl __nw__FUl;
  cmpwi r3, 0;
  beq lbl_8023b8f8;
  lfs f1, unk_80389008;
  li r0, 0;
  stw r0, 8(r1);
  addi r4, r1, 8;
  fmr f2, f1;
  lfs f3, unk_8038900c;
  lfs f4, unk_80389010;
  li r5, 0;
  bl __ct__Q23EGG10ColorFaderFffffQ34nw4r2ut5ColorQ33EGG5Fader7EStatus;
lbl_8023b8f8:
  stw r3, 0x24(r31);
  lwz r31, 0x1c(r1);
  lwz r0, 0x24(r1);
  mtlr r0;
  addi r1, r1, 0x20;
  blr;
  // clang-format on
}
#endif

namespace EGG {

void SceneManager::setupNextSceneID() {
  this->_1C = this->_18;
  this->_18 = this->mNextSceneID;
  this->mNextSceneID = -1;
}

void SceneManager::outgoingParentScene(Scene* pScene) {
  pScene->outgoing_childCreate();
}

}

// Symbol: findParentScene__Q23EGG12SceneManagerFi
// PAL: 0x8023b940..0x8023b980
MARK_BINARY_BLOB(findParentScene__Q23EGG12SceneManagerFi, 0x8023b940, 0x8023b980);
asm UNKNOWN_FUNCTION(findParentScene__Q23EGG12SceneManagerFi) {
  // clang-format off
  nofralloc;
  lwz r3, 0xc(r3);
  li r5, 0;
  lwz r3, 0x20(r3);
  b lbl_8023b968;
lbl_8023b950:
  lwz r0, 0x28(r3);
  cmpw r4, r0;
  bne lbl_8023b964;
  li r5, 1;
  b lbl_8023b970;
lbl_8023b964:
  lwz r3, 0x20(r3);
lbl_8023b968:
  cmpwi r3, 0;
  bne lbl_8023b950;
lbl_8023b970:
  cmpwi r5, 0;
  bnelr;
  li r3, 0;
  blr;
  // clang-format on
}
