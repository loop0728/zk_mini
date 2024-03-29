#pragma once
/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[ID值]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index) 
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress) 
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX() 
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXX->setText("****") 在控件TextXXX上显示文字****
*mButton1->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBar->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

#ifdef SUPPORT_WLAN_MODULE
#include "mi_common_datatype.h"
#include "mi_wlan.h"
#include "wifiInfo.h"



static bool isWifiSupport = true;
static bool isWifiEnable = true;
static WLAN_HANDLE wlanHdl = -1;
static MI_WLAN_OpenParams_t stOpenParam = {E_MI_WLAN_NETWORKTYPE_INFRA};
static MI_WLAN_InitParams_t stParm = {"/config/wifi/wlan.json"};
static MI_WLAN_Status_t  status;
static bool isBootupConnect = false;

class WifiSetupThread : public Thread {
public:
	void setCycleCnt(int cnt, int sleepMs) { nCycleCnt = cnt; nSleepMs = sleepMs; }

protected:
	virtual bool threadLoop() {
		if (!isBootupConnect)
		{
			MI_WLAN_ConnectParam_t *pConnParam = getConnectParam();

			if (MI_WLAN_Init(&stParm) || MI_WLAN_Open(&stOpenParam))
			{
				setWifiSupportStatus(false);
				return false;
			}

			if (isWifiEnable && wlanHdl != -1)
			{
				printf("conn param: id=%d, ssid=%s, passwd=%s\n", wlanHdl, (char*)pConnParam->au8SSId, (char*)pConnParam->au8Password);
				MI_WLAN_Connect(&wlanHdl, pConnParam); // save after connect
				printf("save conn param: id=%d, ssid=%s, passwd=%s\n", wlanHdl);
			}

			isBootupConnect = true;
		}

		if (isWifiEnable && wlanHdl != -1)
		{
			if (nCycleCnt-- > 0)
			{
				MI_WLAN_GetStatus(&status);

				if(status.stStaStatus.state == WPA_COMPLETED)
				{
					setConnectionStatus(true);
					printf("%s %s\n", status.stStaStatus.ip_address, status.stStaStatus.ssid);
					return false;
				}

				if (!nCycleCnt)
					printf("wifi inconnected\n");

				sleep(nSleepMs);
				return true;
			}
		}

		return false;
	}

private:
	int nCycleCnt;
	int nSleepMs;
};

static WifiSetupThread wifiSetupThread;
#endif

static int g_curPageIdx = 0;
/**
 * 注册定时器
 * 在此数组中添加即可
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1->setText("123");
	EASYUICONTEXT->hideStatusBar();

#ifdef SUPPORT_WLAN_MODULE
	// get wifi status from config file
	if (checkProfile() < 0)
	{
		printf("wlan config files error\n");
		return;
	}

	initWifiConfig();
	isWifiSupport = getWifiSupportStatus();
	isWifiEnable = getWifiEnableStatus();
	wlanHdl = getWlanHandle();

	if (isWifiSupport)
	{
		wifiSetupThread.setCycleCnt(200, 50);
		printf("wifiSetupThread run\n");
		wifiSetupThread.run("wifiSetup");
	}
#endif
}

static void onUI_quit() {
#ifdef SUPPORT_WLAN_MODULE
		wifiSetupThread.requestExitAndWait();
		if (wlanHdl != -1)
		{
			MI_WLAN_Disconnect(wlanHdl);
		}

		MI_WLAN_Close();
		sleep(3);
		MI_WLAN_DeInit();
#endif
}


static void onProtocolDataUpdate(const SProtocolData &data) {
    // 串口数据回调接口
}

static bool onUI_Timer(int id){
    //Tips:添加定时器响应的代码到这里,但是需要在本文件的 REGISTER_ACTIVITY_TIMER_TAB 数组中 注册
    //id 是定时器设置时候的标签,这里不要写耗时的操作，否则影响UI刷新,ruturn:[true] 继续运行定时器;[false] 停止运行当前定时器
    return true;
}


static bool onmainActivityTouchEvent(const MotionEvent &ev) {
    // 返回false触摸事件将继续传递到控件上，返回true表示该触摸事件在此被拦截了，不再传递到控件上

	switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//printf("down: x=%d, y=%d\n", ev.mX, ev.mY);

			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			//printf("move: x=%d, y=%d\n", ev.mX, ev.mY);

			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			//printf("up: x=%d, y=%d\n", ev.mX, ev.mY);
			break;
		default:
			break;
	}

    return false;
}
const char* IconTab[]={
		"testTextActivity",
		"testSliderActivity",
		"testButtonActivity",
		"inputtextActivity",
		"waveViewActivity",
		"testpointerActivity",
		"windowActivity",
		"tesListActivity",
		"adActivity",
		"qrcodeActivity",
		"animationActivity",
		"sliderwindowActivity",
		"uartActivity",
		"painterActivity",
		"networkSettingActivity",
		"cameraActivity",
		
};

static void onSlideItemClick_Slidewindow1(ZKSlideWindow *pSlideWindow, int index) {
#ifndef SUPPORT_WLAN_MODULE
	if (!strcmp(IconTab[index], "networkSettingActivity"))
	{
		printf("wifi module is not loaded\n");
		return;
	}
#endif
	EASYUICONTEXT->openActivity(IconTab[index]);
}

static void onSlidePageChange_Slidewindow1(ZKSlideWindow *pSlideWindow, int page) {
	int totalPage = pSlideWindow->getPageSize();
	g_curPageIdx = pSlideWindow->getCurrentPage();
	//printf("Logic: param page is %d, total page is %d, cur page is %d\n", page, totalPage, g_curPageIdx);
	mListview_indicatorPtr->refreshListView();
}

static int getListItemCount_Listview_indicator(const ZKListView *pListView) {
    //LOGD("getListItemCount_Listview_indicator !\n");
    return 2;
}

static void obtainListItemData_Listview_indicator(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ Listview_indicator  !!!\n");
	if (index == g_curPageIdx)
		pListItem->setBackgroundPic("slider_/indicator_focus.png");
	else
		pListItem->setBackgroundPic("slider_/indicator.png");
}

static void onListItemClick_Listview_indicator(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ Listview_indicator  !!!\n");
	int curPageIdx =  g_curPageIdx;
	//printf("click idx is %d, curPageIdx is %d\n", index, g_curPageIdx);

	while (curPageIdx < index)
	{
		mSlidewindow1Ptr->turnToNextPage();
		curPageIdx++;
	}

	while (curPageIdx > index)
	{
		mSlidewindow1Ptr->turnToPrevPage();
		curPageIdx--;
	}
}
