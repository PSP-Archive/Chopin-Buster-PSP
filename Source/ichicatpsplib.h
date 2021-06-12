//########################################################################################
//
//					ichicat PSP Develop Liblary(2009/11/19)
//								  Header File
//
//				Copyright(C) 2009 ichicat.com All right reserved.
//							http://pspd.ichicat.com
//
//########################################################################################

// 標準ヘッダ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// PSP関連ヘッダ
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspaudiolib.h>
#include <pspaudio.h>

//****************************************************************************************
//
//	Callback
//
//****************************************************************************************

//========================================================================================
int ENDFLUG=0;


//========================================================================================
int ExitCallback( int arg1, int arg2, void *common ){
	ENDFLUG=1;
	return 0;
}


//========================================================================================
int CallbackThread( SceSize args, void *argp ){
	int cbid = sceKernelCreateCallback( "Exit Callback", ExitCallback, NULL );
	sceKernelRegisterExitCallback( cbid );

	sceKernelSleepThreadCB();
	return 0;
}


//========================================================================================
int SetupCallbacks( void ){
	int thid = 0;

	thid = sceKernelCreateThread( "update_thread", CallbackThread, 0x11, 0xFA0, 0, 0 );

	if( thid >= 0 ) sceKernelStartThread( thid, 0, 0 );
   
	return thid;
}



//****************************************************************************************
//
//	Screen
//
//****************************************************************************************

//========================================================================================
// VRAM関連
//========================================================================================
//#include <pspge.h>
//#include <pspgu.h>

static unsigned int staticOffset = 0;

//----------------------------------------------------------------------------------------
static unsigned int GetMemorySize( unsigned int width , unsigned int height, unsigned int psm ){

	switch (psm){
		case GU_PSM_T4:
			return (width * height) >> 1;

		case GU_PSM_T8:
			return width * height;

		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;

		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;

		default:
			return 0;
	}
}

//----------------------------------------------------------------------------------------
void* GetStaticVramBuffer(unsigned int width, unsigned int height, unsigned int psm){

	unsigned int memSize = GetMemorySize( width,height, psm );
	void* result = (void*)staticOffset;
	staticOffset += memSize;

	return result;
}

//----------------------------------------------------------------------------------------
void* GetStaticVramTexture(unsigned int width, unsigned int height, unsigned int psm){
	void* result = GetStaticVramBuffer(width,height,psm);
	return (void*)(((unsigned int)result) + ((unsigned int)sceGeEdramGetAddr()));
}



//========================================================================================
// 画面初期化
//========================================================================================

// 定数マクロ
#define BUF_WIDTH 512
#define SCR_WIDTH 480
#define SCR_HEIGHT 272
static unsigned int __attribute__((aligned(16))) list[262144];

//----------------------------------------------------------------------------------------
void ScreenInitM(void* fbp0){

	void* fbp1 = GetStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	void* zbp = GetStaticVramBuffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

	pspDebugScreenInit();

	sceGuInit();
 	sceGuStart(GU_DIRECT,list);

	sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
	sceGuDepthBuffer(zbp,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);

	sceGuEnable(GU_COLOR_TEST); 
	sceGuColorFunc(GU_NOTEQUAL,0,0xffffff); //黒を透明色とする

	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
}



//========================================================================================
// 画面をクリア
//========================================================================================
void ScreenStart(){
		sceGuStart(GU_DIRECT,list);
 		sceGuClearColor(0);
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
}


//========================================================================================
// 実画面に描画
//========================================================================================
void* ScreenRender(){

		sceGuFinish();
		sceGuSync(0,0);

		// VSYNCを待ってVRAMの切り替え
		sceDisplayWaitVblankStart();
		return sceGuSwapBuffers();
}



//****************************************************************************************
//
//	Sprite
//
//****************************************************************************************

//========================================================================================
//	テクスチャ構造体
//========================================================================================
struct VertexTCP{
	float u,v;
	unsigned int color;
	float x,y,z;
};

//========================================================================================
//	座標構造体
//========================================================================================
struct Vector{
	float x,y,z;
};

//========================================================================================
//	スプライトクラス
//========================================================================================
class SpriteM{

	public:
		struct Vector Pos;			// 表示中心座標(x,y,z)
		struct Vector Scale;		// 拡大率(x,y,z)
		float Angle;				// 回転角
		float _Width, _Height;		// 幅と高さ
		float _txx, _txy;				// テクスチャの左上座標
		float _txwidth, _txheight;	// テクスチャの幅と高さ
		unsigned char* _texa;		// テクスチャのアドレス
		int _texsize;				// テクスチャの幅高さのサイズ


	//----------------------------------------------------------------------------------------
	//	スプライト構造体の初期化
	//----------------------------------------------------------------------------------------
	//                幅       高さ     テクスチャ左上X/Y   テクスチャアドレス   テクスチャサイズ
	void Init(float w, float h, float tx, float ty, unsigned char* texa, int texsize){
		Pos.z = 0;

		_Width = w;
		_Height = h;

		Scale.x =1.0f;
		Scale.y =1.0f;
		Scale.z =0.0f;

		Angle = 0;

		_txx=tx;
		_txy=ty;
		_txwidth = w+1;
		_txheight = h;
		_texa = texa;
		_texsize = texsize;
	}


	//----------------------------------------------------------------------------------------
	//	描画
	//----------------------------------------------------------------------------------------
	void Render(){

		struct VertexTCP *pos =  (VertexTCP*)sceGuGetMemory(sizeof(struct VertexTCP)*4);

		int i;
		float pointAngle = atanf((_Height/2.0f)/(_Width/2.0f));

		float texuv[2][4]={
			{ _txx+1 , _txx+_txwidth-1 , _txx+_txwidth-1  , _txx+1            },
			{ _txy   , _txy             , _txy+_txheight , _txy+_txheight }
		};

		for(i=0;i<4;i++){
			pos[i].u=texuv[0][i];
			pos[i].v=texuv[1][i];
			pos[i].color=0xefffffff;
			pos[i].z = Pos.z;
		}

		pos[0].x = 	Pos.x + cosf(pointAngle+Angle)*-1*(1/cosf(pointAngle)*(_Width/2.0f)*Scale.x); 
		pos[0].y =  Pos.y + sinf(pointAngle+Angle)*-1*(1/cosf(pointAngle)*(_Width/2.0f)*Scale.y);
		pos[1].x = 	Pos.x + cosf(pointAngle-Angle)*   (1/cosf(pointAngle)*(_Width/2.0f)*Scale.x); 
		pos[1].y =  Pos.y + sinf(pointAngle-Angle)*-1*(1/cosf(pointAngle)*(_Width/2.0f)*Scale.y);
		pos[2].x = 	Pos.x + cosf(pointAngle+Angle)*   (1/cosf(pointAngle)*(_Width/2.0f)*Scale.x); 
		pos[2].y =  Pos.y + sinf(pointAngle+Angle)*   (1/cosf(pointAngle)*(_Width/2.0f)*Scale.y);
		pos[3].x =	Pos.x + cosf(pointAngle-Angle)*-1*(1/cosf(pointAngle)*(_Width/2.0f)*Scale.x);
		pos[3].y =  Pos.y + sinf(pointAngle-Angle)*   (1/cosf(pointAngle)*(_Width/2.0f)*Scale.y);

		sceGuEnable(GU_TEXTURE_2D);
		sceGuTexMode(GU_PSM_8888,0,0,0);
		sceGuTexImage( 0, _texsize, _texsize, _texsize, _texa );
		sceGuDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, pos);

	}
};

//****************************************************************************************
//
//	Sound
//
//****************************************************************************************

//----------------------------------------------------------------------------------------
//	サウンドの初期化
//----------------------------------------------------------------------------------------
int SoundInit(unsigned int size_wav){
	return sceAudioChReserve( PSP_AUDIO_NEXT_CHANNEL, size_wav/64*64/2/2, PSP_AUDIO_FORMAT_STEREO );
}


//----------------------------------------------------------------------------------------
//	サウンドの再生
//----------------------------------------------------------------------------------------
void SoundPlay(int hSound, unsigned char* sound){
	sceAudioOutputBlocking( hSound , PSP_AUDIO_VOLUME_MAX , sound );
}





//****************************************************************************************
//
//	Controller
//
//****************************************************************************************

//========================================================================================
//	パッドクラス
//========================================================================================
class PadM{

	public:
		int Select;		// SELECTボタン
		int Start;		// STARTボタン
		int Up;			// ↑ボタン 
		int Right;		// →ボタン
		int Down;		// ↓ボタン
		int Left;		// ←ボタン
		int Ltrigger;	// Lボタン
		int Rtrigger;	// Rボタン
		int Triangle;	// △ボタン
		int Circle;		// ○ボタン
		int Cross;		// ×ボタン
		int Square;		// □ボタン
		int Home;		// HOMEボタン
		int Note;		// ♪ボタン
		int Screen;		// ディスプレイボタン
		int VOlUp;		// ボリューム＋ ボタン
		int VolDown;	// ボリューム－ ボタン

		SceCtrlData Controller;


		//----------------------------------------------------------------------------------------
		//	コントローラの初期化
		//----------------------------------------------------------------------------------------
		void Init(){
			sceCtrlSetSamplingCycle(0);
			sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		}

		//----------------------------------------------------------------------------------------
		//	コントローラ値の取得
		//----------------------------------------------------------------------------------------
		void Get(){

			sceCtrlPeekBufferPositive( &Controller, 1 );	

			if( Controller.Buttons & PSP_CTRL_SELECT   ) Select=1;	 else Select=0;		// SELECTボタン
			if( Controller.Buttons & PSP_CTRL_START    ) Start=1;	 else Start=0;		// STARTボタン
			if( Controller.Buttons & PSP_CTRL_UP       ) Up=1;		 else Up=0;			// ↑ボタン 
			if( Controller.Buttons & PSP_CTRL_RIGHT    ) Right=1;	 else Right=0;		// →ボタン
			if( Controller.Buttons & PSP_CTRL_DOWN     ) Down=1;	 else Down=0;		// ↓ボタン
			if( Controller.Buttons & PSP_CTRL_LEFT     ) Left=1;	 else Left=0;		// ←ボタン
			if( Controller.Buttons & PSP_CTRL_LTRIGGER ) Ltrigger=1; else Ltrigger=0;	// Lボタン
			if( Controller.Buttons & PSP_CTRL_RTRIGGER ) Rtrigger=1; else Rtrigger=0;	// Rボタン
			if( Controller.Buttons & PSP_CTRL_TRIANGLE ) Triangle=1; else Triangle=0;	// △ボタン
			if( Controller.Buttons & PSP_CTRL_CIRCLE   ) Circle=1;	 else Circle=0;		// ○ボタン
			if( Controller.Buttons & PSP_CTRL_CROSS    ) Cross=1;	 else Cross=0;		// ×ボタン
			if( Controller.Buttons & PSP_CTRL_SQUARE   ) Square=1;	 else Square=0;		// □ボタン
			if( Controller.Buttons & PSP_CTRL_HOME     ) Home=1;	 else Home=0;		// HOMEボタン
			if( Controller.Buttons & PSP_CTRL_NOTE     ) Note=1;	 else Note=0;		// ♪ボタン
			if( Controller.Buttons & PSP_CTRL_SCREEN   ) Screen=1;	 else Screen=0;		// ディスプレイボタン
			if( Controller.Buttons & PSP_CTRL_VOLUP    ) VOlUp=1;	 else VOlUp=0;		// ボリューム＋ ボタン
			if( Controller.Buttons & PSP_CTRL_VOLDOWN  ) VolDown=1;	 else VolDown=0;	// ボリューム－ ボタン

		}
};
