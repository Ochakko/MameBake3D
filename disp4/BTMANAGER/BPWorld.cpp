#include "BPWorld.h"
#include "GlutStuff.h"
#include "BulletCollision/CollisionShapes/btStaticPlaneShape.h"
#include "BulletDynamics/ConstraintSolver/btContactSolverInfo.h"
#include <iostream>
#include <cstdlib>

#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

#include <coef.h>

//#define USE_PROF
#ifdef USE_PROF
#include <fstream>
std::ofstream g_prof("sim_prof.txt");
#endif

#include "..\..\ChatAnim\\GColiIDDlg.h"

using namespace std;


extern int g_previewFlag;			// プレビューフラグ
extern int gNumClampedCcdMotions;

btScalar G_ACC = 9.8; // 重力加速度

static const btScalar DEFAULT_TIME_STEP = 1.0/100.0; // 刻み時間のデフォルト値
//static const btScalar G_ACC = 0.0; // 重力加速度
static const btScalar ERP = 0.0;   // ERPは"Error Reduction Parameter"の略

static btConstraintSolver* sConstraintSolvers = 0;


////////////////////////////////////////////////////////////////////////////////
/**
 *  コンストラクタ
 */
BPWorld::BPWorld(D3DXMATRIX wmat, char* winTitle, int winWidth, int winHeight, MonitoringFunc monFnPtr, KeyboardFunc keyFnPtr)
{
  m_wmat = wmat;
  m_softBodyRequested = false;
  init(winTitle, winWidth, winHeight, monFnPtr, keyFnPtr);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  コンストラクタ
 */
BPWorld::BPWorld(char* winTitle, int winWidth, int winHeight, WorldType wldType, MonitoringFunc monFnPtr, KeyboardFunc keyFnPtr)
{
  if(wldType == BPWorld::RIGID_BODY_ONLY){
    m_softBodyRequested = false;
  }else if(wldType == BPWorld::SOFT_BODY_INCLUDED){
    m_softBodyRequested = true;
  }
  init(winTitle, winWidth, winHeight, monFnPtr, keyFnPtr);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  初期化処理（2種類のコンストラクタの共通部分）
 */
void BPWorld::init(char* winTitle, int winWidth, int winHeight, MonitoringFunc monFnPtr, KeyboardFunc keyFnPtr)  
{
	m_groupid = 1;
	m_myselfflag = 1;
	m_coliids.clear();
	m_coliids.push_back( 2 );

	m_gplaneh = -1.0f;
	m_gdisph = 0.0f;
	m_gplanesize = D3DXVECTOR2( 5.0f, 5.0f );
	m_gplanedisp = 1;

	m_restitution = 0.5f;
	m_friction = 0.5f;

  m_collisionG = 0;
  m_rigidbodyG = 0;
  m_broadphase = NULL;
  m_dispatcher = NULL;
  m_solver = NULL;
  m_collisionConfiguration = NULL;
  m_timeStep    = DEFAULT_TIME_STEP;
  //m_fixedTimeStepEnabled = false;
  m_fixedTimeStepEnabled = true;
  m_currentTime = 0;
  m_winTitle  = winTitle;
  m_winWidth  = winWidth;
  m_winHeight = winHeight;
  m_doMonitoring = monFnPtr;
  m_checkKey = keyFnPtr;
  
  // 動力学シミュレーションの準備
  initPhysics();

  //// 描画設定
  // 描画機能を追加するクラスを使うための呼び出し
  overrideGLShapeDrawer(new BPWorld::NewShapeDrawer());
  setTexturing(true);
  setShadows(true);
  //setCameraDistance(20);
//void BPWorld::setCamera(float ptx, float pty, float ptz,
//                        float dist, float azi, float ele,
//                        float upx, float upy, float upz)
  setCamera( 0.0f, 100.0f, -200.0f, 2000.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
  //setCameraDistance(100);
  m_dynamicsWorld->setDebugDrawer(&m_debugDrawer);
  setDebugMode(btIDebugDraw::DBG_NoHelpText |         // ヘルプテキスト表示を消す
               0);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  動力学シミュレーションの準備を行う
 */
int BPWorld::RemakeG()
{
	if( m_rigidbodyG ){
		btCollisionObject* obj = (btCollisionObject*)m_rigidbodyG;
		if(m_rigidbodyG->getMotionState()){
			delete m_rigidbodyG->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}
	
	if( m_collisionG ){
		delete m_collisionG;
		m_collisionG = 0;
	}

/////////////////////////

	// 地面（ワールド座標系の Z-X 平面）
//	m_collisionG = new btStaticPlaneShape(btVector3(0, 1, 0), m_gplaneh );
//	btTransform transform;
//	transform.setIdentity();
//	transform.setOrigin( btVector3( btScalar( 0.0f ), btScalar( 0.0f ), btScalar( 0.0f ) ) );

	m_collisionG = new btStaticPlaneShape(btVector3(0.0, 1.0, 0.0), m_gplaneh );
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin( btVector3( 0.0f, 0.0f, 0.0f ) );


	// DemoApplication::localCreateRigidBody()はbtRigidBodyの生成，
	// そのbtRigidBodyのm_dynamicsWorldへの登録を行う．
	// 指定された形状に応じた慣性モーメント行列の計算も行われる．
	m_rigidbodyG = LCreateRigidBody(0, // 質量, 0にすると固定された静止物体として扱われる
							transform,
							m_collisionG);
	m_rigidbodyG->setRestitution(m_restitution); // 反発係数．初期値が0なので反発してほしい物体全てに明示的に設定する  
	m_rigidbodyG->setFriction(m_friction);

	//m_rigidbodyG->setWorldTransform( transform );

	return 0;
}


void BPWorld::initPhysics()
{
  if(m_softBodyRequested){
    initWorldForSoftBody();
  }else{
    initWorldForRigidOnly();
  }
  m_dynamicsWorld->setGravity(btVector3(0, -G_ACC, 0)); // 重力加速度の設定
  btContactSolverInfo& slvInfo = m_dynamicsWorld->getSolverInfo();
  //slvInfo.m_numIterations = 10;  
  slvInfo.m_erp  = ERP;
  //slvInfo.m_erp2 = 0.0;
  //slvInfo.m_globalCfm = 1.0;
  //slvInfo.m_linearSlop = 0.0;
  //slvInfo.m_warmstartingFactor = 0.85;
  slvInfo.m_splitImpulse = 1e10;

  //slvInfo.m_restitution = 0.0;
  //slvInfo.m_restingContactRestitutionThreshold = 2;
  //slvInfo.m_solverMode |= SOLVER_USE_WARMSTARTING;
  //slvInfo.m_solverMode |= SOLVER_RANDMIZE_ORDER;
 
  RemakeG();

//	sConstraintSolvers = new btSequentialImpulseConstraintSolver();
//	m_dynamicsWorld->setConstraintSolver( sConstraintSolvers );

  // 三角メッシュ（btGImpactMeshShape）形状の物体の衝突判定のためには下記が必要
  // （btBoxShape等の基本形状の物体の衝突判定だけならば不要）
  btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher);
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  剛体のみのシミュレーションを行う場合のワールドの設定
 */
void BPWorld::initWorldForRigidOnly()
{
  ///collision configuration contains default setup for memory, collision setup
  m_collisionConfiguration = new btDefaultCollisionConfiguration();

  ///use the default collision dispatcher.
  // For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
  m_broadphase = new btDbvtBroadphase();

  ///the default constraint solver.
  // For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
  m_solver = new btSequentialImpulseConstraintSolver;
  //m_solver = new btParallelBatchConstraintSolver;
  m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver,
                                                m_collisionConfiguration);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  剛体および柔軟体のシミュレーションを行う場合のワールドの設定
 */
void BPWorld::initWorldForSoftBody()
{
  btVector3 worldAabbMin(-1000, -1000, -1000);
  btVector3 worldAabbMax(1000, 1000, 1000);
  const int MAX_PROXIES = 32766;

  m_broadphase = new btAxisSweep3(worldAabbMin, worldAabbMax, MAX_PROXIES);
  m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
  m_solver = new btSequentialImpulseConstraintSolver();

  m_dynamicsWorld = new btSoftRigidDynamicsWorld(m_dispatcher,
                                                 m_broadphase,
                                                 m_solver,
                                                 m_collisionConfiguration);
  m_dynamicsWorld->getDispatchInfo().m_enableSPU = true;
  
  m_softBodyWorldInfo.m_broadphase = m_broadphase;
  m_softBodyWorldInfo.m_dispatcher = m_dispatcher;
  
  m_softBodyWorldInfo.air_density   = 1.2;
  m_softBodyWorldInfo.water_density = 0;
  m_softBodyWorldInfo.water_offset  = 0;
  m_softBodyWorldInfo.water_normal  = btVector3(0,0,0);
  m_softBodyWorldInfo.m_gravity.setValue(0, -G_ACC, 0);

  
  m_softBodyWorldInfo.m_sparsesdf.Initialize();
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  デストラクタ．
 *  確保したメモリ（btCollisionShape, etc.）を解放する
 */
BPWorld::~BPWorld()
{

  if( m_rigidbodyG ){
    btCollisionObject* obj = (btCollisionObject*)m_rigidbodyG;
    if(m_rigidbodyG->getMotionState()){
      delete m_rigidbodyG->getMotionState();
    }
    m_dynamicsWorld->removeCollisionObject( obj );
	delete obj;
  }
	
// cleanup in the reverse order of creation/initialization
  // remove the rigid bodies from the dynamics world and delete them
  for(int i = m_dynamicsWorld->getNumCollisionObjects()-1; i >= 0; i--){
    btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    if (body && body->getMotionState()){
      delete body->getMotionState();
    }
    m_dynamicsWorld->removeCollisionObject( obj );
	delete obj;
	//m_dynamicsWorld->removeRigidBody( body );
    //delete body;
  }

  if( m_collisionG ){
	  delete m_collisionG;
	  m_collisionG = 0;
  }

  // delete collision shapes
  for(int j = 0; j < m_collisionShapes.size(); j++){
    btCollisionShape* shape = m_collisionShapes[j];
    delete shape;
  }

  // delete constraints
  for(int i = m_dynamicsWorld->getNumConstraints()-1; i >= 0; i--){
    btTypedConstraint* constraint = m_dynamicsWorld->getConstraint(i);
    m_dynamicsWorld->removeConstraint(constraint);
    delete constraint;
  }

  delete m_dynamicsWorld;
  delete m_solver;
  //delete sConstraintSolvers;
  delete m_broadphase;
  delete m_dispatcher;
  delete m_collisionConfiguration;  
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  ウィンドウを表示して，シミュレーションを開始する
 */

int BPWorld::start()
{
  int  argCount = 1;
  char *argVec[] = {m_winTitle};
  return glutmain(argCount, argVec, m_winWidth, m_winHeight, m_winTitle, this);
  //return 0;
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  物体の力学データを作成し，ワールドに登録する
 *
 *  @param mass 作成する物体の質量
 *  @param initTransform 作成する物体の初期位置・姿勢
 *  @param shape 作成する物体の形状
 */
/***
btRigidBody* BPWorld::createRigidBody(btScalar mass,
                                      const btTransform& initTransform,
                                      btCollisionShape* shape)
{
  // 二重解放を避けるため，まだ登録されていない形状データかどうか
  // 確認してからshapeを配列に追加する．
  // findLinearSearch()で見つからない場合は，返り値は配列のサイズになる
  if(m_collisionShapes.findLinearSearch(shape) == m_collisionShapes.size()){
    m_collisionShapes.push_back(shape); // 終了時に解放するために保存
  }//else{
  //std::cout << "Duplicate element found" << std::endl;
  //}
  

  // 指定された物体を作ってワールドに登録する
  return localCreateRigidBody(mass, initTransform, shape); 
}
***/

////////////////////////////////////////////////////////////////////////////////
/**
 *  ジョイント（constraint）をワールドに登録する
 */
void BPWorld::addConstraint(btTypedConstraint *constraint, 
                            bool disableCollisionsBetweenLinkedBodies)
{
  m_dynamicsWorld->addConstraint(constraint,
                                 disableCollisionsBetweenLinkedBodies);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  ジョイント（constraint）をワールドから削除する
 */
void BPWorld::removeConstraint(btTypedConstraint *constraint)
{
  m_dynamicsWorld->removeConstraint(constraint);
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  シミュレーションのメインループから呼ばれるメンバ関数（コールバック）
 */
void BPWorld::clientMoveAndDisplay()
{
  if(m_doMonitoring != NULL){
    m_doMonitoring(m_currentTime); // ユーザが定義したモニタリング関数を呼び出す
  }
  
  //if( g_previewFlag == 4 ){
	  // シミュレーションを進める 
	  if(m_dynamicsWorld != NULL){
		if(m_fixedTimeStepEnabled){
		  const int NO_SUBDIVISION = 0;
		  m_dynamicsWorld->stepSimulation(m_timeStep, NO_SUBDIVISION);
		  m_currentTime += m_timeStep;
		}else{
		  const btScalar USEC_TO_SEC = 1.0/(1000*1000);
		  btScalar dt = m_clock.getTimeMicroseconds()*USEC_TO_SEC; // 前フレーム描画からの経過時間の測定
		 //const int MAX_SUBSTEPS = 4;
		  //const btScalar SUBSTEP_LEN = 0.05;//0.005; // 0.001; //1.0/60; // 0.01;

//		const int MAX_SUBSTEPS = 8;
//		const btScalar SUBSTEP_LEN = 0.025;//0.005; // 0.001; //1.0/60; // 0.01;

//		const int MAX_SUBSTEPS = 16;
//		const btScalar SUBSTEP_LEN = 0.0125;

		const int MAX_SUBSTEPS = 20;
		const btScalar SUBSTEP_LEN = 0.0020;

		  m_clock.reset(); // 次回の測定の準備
	#ifdef USE_PROF
		  static int n = 0;
		  n += m_dynamicsWorld->stepSimulation(dt, MAX_SUBSTEPS, SUBSTEP_LEN); // 実経過時間に合わせてシミュレーションを進める
		  g_prof << n << " " << dt <<" " << (m_currentTime + dt)/n << "\n";
		  // 理想的には(m_currentTIme + dt)/n → SUBSTEP_LEN
	#else      
		  m_dynamicsWorld->stepSimulation(dt, MAX_SUBSTEPS, SUBSTEP_LEN); // 実経過時間に合わせてシミュレーションを進める
	#endif      
		  m_currentTime += dt;
		}
    
		m_softBodyWorldInfo.m_sparsesdf.GarbageCollect();
	  }
  //}

//  displayCallback();
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  シミュレーション停止状態のとき（'i'キーが押されたとき）に
 *  シーンの描画のために呼ばれる関数（コールバック）
 */
void BPWorld::displayCallback(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderme();
  m_dynamicsWorld->debugDrawWorld();
  swapBuffers();
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  キーボード入力処理のために呼ばれる関数（コールバック）
 */
void BPWorld::keyboardCallback(unsigned char key, int x, int y)
{
  this->DemoApplication::keyboardCallback(key, x, y);

  switch(key){
  case 'e':
    //btVector3 pc = getCameraPosition();
    //printf("cam pos: %f, %f, %f\n", pc.x(), pc.y(), pc.z());
    printf("cam. dist.: %f\n", getCameraDistance());
    printf("cam. azi.: %f\n", m_azi);
    printf("cam. ele.: %f\n", m_ele);
    printf("cam. up:   %f, %f, %f\n", m_cameraUp[0], m_cameraUp[1], m_cameraUp[2]);
    printf("cam. target pos.: %f, %f, %f\n",
           m_cameraTargetPosition[0],
           m_cameraTargetPosition[1],
           m_cameraTargetPosition[2]);
    break;
  }
  
  if(m_checkKey != NULL){
    m_checkKey(key, x, y);
  }
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  リセット時（スペースキーが押されたときなど）の処理
 */
void BPWorld::clientResetScene()
{
  this->DemoApplication::clientResetScene();
  m_currentTime = 0;
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  ジョイントの可動範囲の表示を有効にする
 */
void BPWorld::enableConstraintDisplay(bool onOff)
{
  const int CONSTRAINT_FLAGS =
    btIDebugDraw::DBG_DrawContactPoints |  
    btIDebugDraw::DBG_DrawConstraints | 
    btIDebugDraw::DBG_DrawConstraintLimits | // ジョイントの上限・下限の表示
    0;
  
  if(onOff){
    setDebugMode(getDebugMode() | CONSTRAINT_FLAGS);
  }else{
    setDebugMode(getDebugMode() & ~CONSTRAINT_FLAGS);
  }
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  柔軟物体をワールドに追加する
 */
void BPWorld::addSoftBody(btSoftBody* sb)
{
  ((btSoftRigidDynamicsWorld*)m_dynamicsWorld)->addSoftBody(sb);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  デモの構成上，このメンバ関数が各物体を描画する際に呼び出される．
 *
 *  デモのGL_ShapeDrawerでは描けない物体（平面など）を描く機能を追加する
 *
 *  @param transMatrix 描画対象物体の位置・姿勢を表す行列
 *  @param shape 描画対象物体の形状
 *  @param color 色を表すRGB値
 *  @param debugMode デバッグモードを表すフラグ
 *  @param worldBoundsMin 描画対象領域を表す直方体(axis-aligned bounding box)の最小値側頂点に対応する点
 *  @param worldBoundsMax 描画対象領域を表す直方体(axis-aligned bounding box)の最大値側頂点に対応する点
 */
void BPWorld::NewShapeDrawer::drawOpenGL(btScalar* transMatrix,
                                         const btCollisionShape* shape,
                                         const btVector3& color, int debugMode,
                                         const btVector3& worldBoundsMin,
                                         const btVector3& worldBoundsMax)
{
    // 既存の描画関数で描ける物体は，既存の描画関数で描く
  this->GL_ShapeDrawer::drawOpenGL(transMatrix, shape, color, debugMode,
                                   worldBoundsMin, worldBoundsMax);

  if(m_textureenabled){ // テクスチャマッピングを行う場合
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_texturehandle);
  }else{
    glDisable(GL_TEXTURE_2D);
  }

  // 平面（を表す正方形）を描く
  if(shape->getShapeType() == STATIC_PLANE_PROXYTYPE){ 
    const btStaticPlaneShape *planeShape = static_cast<const btStaticPlaneShape*>(shape);
      
    // 平面の方程式は (n,r) = d. ここで r は3次元位置ベクトル変数．(n,r)は内積
    btVector3 n = planeShape->getPlaneNormal();   // 平面の法線ベクトル
    btScalar  d = planeShape->getPlaneConstant(); // 原点から平面に下ろした垂線の長さ
    const btScalar L = 600;//40.0; // 平面を表す正方形一辺の長さの半分
    btVector3 p0 = n * d; // 原点から平面に下ろした垂線の足を指す位置ベクトル
    btVector3 v0, v1; // nに垂直かつ，互いに直交するベクトル
    btPlaneSpace1(n, v0, v1); // v0, v1を求める．btPlaneSpace1() はbtVector3.hで定義されている．

    // 平面を表す矩形の四隅それぞれの位置p1, p2, p3, p4を計算する
    btVector3 p1 =  p0 + ( v0 + v1)*L;
    btVector3 p2 =  p0 + (-v0 + v1)*L;
    btVector3 p3 =  p0 + (-v0 - v1)*L;
    btVector3 p4 =  p0 + ( v0 - v1)*L;

    glBegin(GL_QUADS);
    {
      glNormal3f(n[0], n[1], n[2]); // この平面について照光処理を行うため
      
      glVertex3f(p1[0], p1[1], p1[2]);  
      glVertex3f(p2[0], p2[1], p2[2]);
      glVertex3f(p3[0], p3[1], p3[2]);
      glVertex3f(p4[0], p4[1], p4[2]);
    }
    glEnd();
  }

  if(m_textureenabled){
    glDisable(GL_TEXTURE_2D);
  }
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  カメラの位置，姿勢を設定する
 *
 *  @param ptx 注視点位置 [ptx, pty, ptz] の x成分
 *  @param pty 注視点位置 [ptx, pty, ptz] の y成分
 *  @param ptz 注視点位置 [ptx, pty, ptz] の z成分
 *  @param dist 注視点からカメラ視点までの距離 
 *  @param azi  方位角（azimuth） [deg]  ワールド座標系Z軸と，視線方向ベクトルのXZ平面への射影がなす角度
 *  @param ele  仰角（elevation） [deg]  XZ平面と視線方向ベクトルがなす角度
 *  @param upx 天井方向ベクトル [upx, upy, upz] の x成分（cf. GLのgluLookAt()）
 *  @param upy 天井方向ベクトル [upx, upy, upz] の y成分（cf. GLのgluLookAt()）
 *  @param upz 天井方向ベクトル [upx, upy, upz] の z成分（cf. GLのgluLookAt()）
 */
void BPWorld::setCamera(float ptx, float pty, float ptz,
                        float dist, float azi, float ele,
                        float upx, float upy, float upz)
{
  // これらのカメラ関連のメンバはDemoApplication.hで定義されている．
  m_cameraTargetPosition = btVector3(ptx, pty, ptz);
  setCameraDistance(dist);
  m_azi = azi;
  m_ele = ele;
  setCameraUp(btVector3(upx, upy, upz));
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  
 *  カメラの状態（投影行列およびモデル・ビュー行列）を更新する．
 *  BulletPhysicsライブラリ ver. 2.75の実装だとカメラの動きが
 *    |m_ele| = +90 deg付近で不連続（おそらく誤り）のため実装しなおす．
 */
void BPWorld::updateCamera()
{
  btScalar rele = btRadians(m_ele); // [rad]
  btScalar razi = btRadians(m_azi); // [rad] 
  btQuaternion rot(m_cameraUp, razi);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();


  btVector3 eyePos(0, 0, -m_cameraDistance);

  btVector3 forward(eyePos[0], eyePos[1], eyePos[2]);
  if(forward.length2() < SIMD_EPSILON){
    forward.setValue(1.f,0.f,0.f);
  }
  btVector3 right = m_cameraUp.cross(forward);
  btQuaternion roll(right, -rele);

  eyePos = quatRotate(rot * roll, eyePos);

  m_cameraPosition = eyePos + m_cameraTargetPosition;
    
  if(m_glutScreenWidth == 0 && m_glutScreenHeight == 0){
    return;
  }
  btScalar aspect;
  btVector3 extents;

  if(m_glutScreenWidth > m_glutScreenHeight){
    aspect = m_glutScreenWidth / (btScalar)m_glutScreenHeight;
    extents.setValue(aspect * 1.0f, 1.0f,0);
  }else{
    aspect = m_glutScreenHeight / (btScalar)m_glutScreenWidth;
    extents.setValue(1.0f, aspect*1.f,0);
  }

	
  if(m_ortho){
    // reset matrix
    glLoadIdentity();
		
		
    extents *= m_cameraDistance;
    btVector3 lower = m_cameraTargetPosition - extents;
    btVector3 upper = m_cameraTargetPosition + extents;
    //gluOrtho2D(lower.x, upper.x, lower.y, upper.y);
    glOrtho(lower.getX(), upper.getX(), lower.getY(), upper.getY(),-1000,1000);
		
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glTranslatef(100,210,0);
  }else{
    if(m_glutScreenWidth > m_glutScreenHeight){
      glFrustum (-aspect, aspect, -1.0, 1.0, 1.0, 10000.0);
      //glFrustum (-aspect * m_frustumZNear, aspect * m_frustumZNear, -m_frustumZNear, m_frustumZNear, m_frustumZNear, m_frustumZFar);
    }else {
      glFrustum (-1.0, 1.0, -aspect, aspect, 1.0, 10000.0);
      //glFrustum (-aspect * m_frustumZNear, aspect * m_frustumZNear, -m_frustumZNear, m_frustumZNear, m_frustumZNear, m_frustumZFar);
    }

    btVector3 up;
    if(SIMD_HALF_PI <= fabs(rele)  &&  fabs(rele) <= (3.0/2.0)*SIMD_PI){
      up = - m_cameraUp;
    }else{
      up = m_cameraUp;
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(m_cameraPosition[0], m_cameraPosition[1], m_cameraPosition[2], 
              m_cameraTargetPosition[0], m_cameraTargetPosition[1], m_cameraTargetPosition[2], 
              up[0], up[1], up[2]);
  }
  
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  マウスドラッグ時の処理
 *  BulletPhysicsライブラリ ver. 2.75の実装だとカメラの動きが
 *    |m_azi| = +180 deg付近で不連続（おそらく誤り）のため実装しなおす．
 */
void BPWorld::mouseMotionFunc(int x, int y)
{
  extern float gOldPickingDist; // 実体の定義はDemoApplication.cppにあり
  
  // ピッキング時の処理
  if(m_pickConstraint){
    //move the constraint pivot
    btPoint2PointConstraint* p2p = static_cast<btPoint2PointConstraint*>(m_pickConstraint);
    if(p2p){
      //keep it at the same picking distance
      btVector3 newRayTo = getRayTo(x,y);
      btVector3 rayFrom;
      btVector3 oldPivotInB = p2p->getPivotInB();
      btVector3 newPivotB;
      if (m_ortho){
        newPivotB = oldPivotInB;
        newPivotB.setX(newRayTo.getX());
        newPivotB.setY(newRayTo.getY());
      }else{
        rayFrom = m_cameraPosition;
        btVector3 dir = newRayTo-rayFrom;
        dir.normalize();
        dir *= gOldPickingDist;

        newPivotB = rayFrom + dir;
      }
      p2p->setPivotB(newPivotB);
    }

  }

  float dx, dy;
  dx = x - m_mouseOldX;
  dy = y - m_mouseOldY;


  // カメラの移動
  // （ALT (or CTRL) を押したままドラッグしているときの処理）
  if(m_modifierKeys & BT_ACTIVE_ALT){
    if(m_mouseButtons & 2){
      btVector3 hor  = getRayTo(0,0) - getRayTo(1,0);
      btVector3 vert = getRayTo(0,0) - getRayTo(0,1);
      btScalar multiplierX = 0.01;
      btScalar multiplierY = 0.01;
      if(m_ortho){
        multiplierX = 1;
        multiplierY = 1;
      }

      m_cameraTargetPosition += hor* dx * multiplierX;
      m_cameraTargetPosition += vert* dy * multiplierY;
    }

    if(m_mouseButtons & 1){
      m_azi += dx * 0.2;
      m_azi = fmodf(m_azi, 360.f);
      m_ele += dy * 0.2;
      m_ele = fmodf(m_ele, 360.f);
    }else if(m_mouseButtons & 4){
      m_cameraDistance -= dy * 0.2f;
      if(m_cameraDistance < SIMD_EPSILON){
        m_cameraDistance = SIMD_EPSILON;
      }
    } 
  }


  m_mouseOldX = x;
  m_mouseOldY = y;
  updateCamera();
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  （接触ジョイントを含む）ジョイントについてのERPのデフォルト値を指定する
 *
 *  @param erp Error Reduction Parameter (ERP)
 */
void BPWorld::setGlobalERP(btScalar erp)
{
  if(m_dynamicsWorld != NULL){
    btContactSolverInfo& slvInfo = m_dynamicsWorld->getSolverInfo();
    slvInfo.m_erp = erp;
  }
}

btRigidBody* BPWorld::LCreateRigidBody( btScalar mass, const btTransform& startTransform, btCollisionShape* shape )
{
	_ASSERT( shape );

	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia( mass, localInertia );

	btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
		
	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, shape, localInertia );
	btRigidBody* body = new btRigidBody( rbInfo );

body->setRestitution( 1.0 );
body->setFriction( 0.5 );

	int coliid = GetColiID();

	m_dynamicsWorld->addRigidBody(body, m_groupid, coliid);

	//DWORD curflag = body->getCollisionFlags();
	//m_rigidbodyG->setCollisionFlags( curflag | btCollisionObject::CF_KINEMATIC_OBJECT);
	//body->setCollisionFlags( curflag | btCollisionObject::CF_STATIC_OBJECT);
	//srcbto->m_rigidbody->setActivationState(DISABLE_DEACTIVATION);
	//srcbto->m_rigidbody->setActivationState(WANTS_DEACTIVATION);
	//srcbto->m_rigidbody->setActivationState(DISABLE_SIMULATION);
	//CF_STATIC_OBJECT

	//body->setGravity( btVector3( 0.0f, 0.0f, 0.0f ) );
	//body->applyGravity();



	return body;
}

int BPWorld::GetColiID()
{
	int retid = 0;

	int idnum = m_coliids.size();
	int ino;
	for( ino = 0; ino < idnum; ino++ ){
		int curid = 1 << (m_coliids[ ino ] - 1);
		retid |= curid;
	}

	if( m_myselfflag == 1 ){
		int curid = 1 << (m_groupid - 1);
		retid |= curid;
	}

	return retid;
}

/***
void	DemoApplication::clientResetScene()
{
	removePickingConstraint();

#ifdef SHOW_NUM_DEEP_PENETRATIONS
	gNumDeepPenetrationChecks = 0;
	gNumGjkChecks = 0;
#endif //SHOW_NUM_DEEP_PENETRATIONS

	gNumClampedCcdMotions = 0;
	int numObjects = 0;
	int i;

	if (m_dynamicsWorld)
	{
		int numConstraints = m_dynamicsWorld->getNumConstraints();
		for (i=0;i<numConstraints;i++)
		{
			m_dynamicsWorld->getConstraint(0)->setEnabled(true);
		}
		numObjects = m_dynamicsWorld->getNumCollisionObjects();
	
		///create a copy of the array, not a reference!
		btCollisionObjectArray copyArray = m_dynamicsWorld->getCollisionObjectArray();

		


		for (i=0;i<numObjects;i++)
		{
			btCollisionObject* colObj = copyArray[i];
			btRigidBody* body = btRigidBody::upcast(colObj);
			if (body)
			{
				if (body->getMotionState())
				{
					btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
					myMotionState->m_graphicsWorldTrans = myMotionState->m_startWorldTrans;
					body->setCenterOfMassTransform( myMotionState->m_graphicsWorldTrans );
					colObj->setInterpolationWorldTransform( myMotionState->m_startWorldTrans );
					colObj->forceActivationState(ACTIVE_TAG);
					colObj->activate();
					colObj->setDeactivationTime(0);
					//colObj->setActivationState(WANTS_DEACTIVATION);
				}
				//removed cached contact points (this is not necessary if all objects have been removed from the dynamics world)
				if (m_dynamicsWorld->getBroadphase()->getOverlappingPairCache())
					m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(colObj->getBroadphaseHandle(),getDynamicsWorld()->getDispatcher());

				btRigidBody* body = btRigidBody::upcast(colObj);
				if (body && !body->isStaticObject())
				{
					btRigidBody::upcast(colObj)->setLinearVelocity(btVector3(0,0,0));
					btRigidBody::upcast(colObj)->setAngularVelocity(btVector3(0,0,0));
				}
			}

		}

		///reset some internal cached data in the broadphase
		m_dynamicsWorld->getBroadphase()->resetPool(getDynamicsWorld()->getDispatcher());
		m_dynamicsWorld->getConstraintSolver()->reset();

	}

}
***/