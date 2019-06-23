#include "stdafx.h"
#include "sysdef.h"
#include "math.h"
#include "e_gvar.h"
#include "e_uibas.h"
#include "e_math3d.h"
#include "e_curve.h"
#include "e_surface.h"
#include "e_nc.h"
#include "e_datum.h"
#include "e_system.h"

#include "resource.h"
#include "ToolPage.h"
#include "MillParaPage.h"
#include "ncpktpage.h"
#include "ncplungepage.h"
#include "ncpktproSheet.h"
#include "Nc.h"
//ZERO = 1.0E-5
#define CCW   1    //
#define CW    -1     //
#define EP  10e-1
#define EL  10e-1
#define UPER   10e2
#define LINEZ  10e-2
#define LL  0  //line line bisector
#define LC  1  //line circle bisector 
#define CC  2  //circle circle bisector
#define CCP 3  //concenter circle circle bisector
#define LLP 4  //parallel line line bisector
#define CIC 5  //circle in circle bisector //ellipse
#define INFINITY  1  //
#define LIMITED  0  //
#define STEP  0.2//画平分线的步长
#define SINGLE  0
#define MULI    1
#define VCOMMON 1
#define CLOSED  2
#define SINGLEPLF  0
#define MULIPLF    1
#define ISLAND     0
#define LOOP       1
#define ERRO      -100
#define REFLEX    -1
#define UNREFLEX   1
///////////////////////
static int myflag=0;
typedef struct bisector{
	double	t1,t2,tmin,tmax;
	int		infinite;//limitations/INFINITY/LIMITLINE
	int     bitype;//类型LC/CC/LL
	int     limit;
	double	x1,x2,x3,x4,x5,x6,x7,x8;
	double	y1,y2,y3,y4,y5,y6,y7,y8;
	double  a1,a2,a3,a4;
	int  	m1,m2;
}VBNS,*PVBNS;//表示单个角平分线
typedef struct contournodesingle{
	int type;//直线或是弧
	PRIMITIVE element;//PRIMITIVE
	int reflex;//if 1,means the reflex node;
	int clockwise;//，段的方向 CCW OR CW
}VCNS,*PVCNS;//?//单个轮廓线

typedef struct contournodeinform{
	PVCNS cns;
	struct contournodeinform *ccw,*cw;
}VCNI,*PVCNI;//一组轮廓线
typedef struct besectorinform{
	PVBNS bns;//PVBNS
	PVCNS selfcns,othercns;//PVCNs
	int   flag;//使用过？0/1 
}VBNI,*PVBNI;//单个角平分线，有对应轮廓线的信息

typedef struct bisectorlinkinform {
	PVBNI bni;//PVBNI
	int clockwise;//角平分线的走向，0（顺）/1（逆）
	struct bisectorlinkinform *next,*last;//struct bisectorlinkinform
}VBLI,*PVBLI;//一组角平分线

typedef struct contournodebisectorlink{
	PVCNS cns;//PVCNI，节点
	PVBLI bli;//ccw direction;PVBLI
	int   flag;//是否是终节点？-1/1
	struct contournodebisectorlink *ccw,*cw;
}VCNBL,*PVCNBL;//一个VORONOI 单元

typedef struct profilelef{
	PVCNBL scnbl,ecnbl;//PVCNBL，一个含有节点和角平分线的voronoi 结构
	int    type;//单节点否？SINGLEPLF/MULIPLF
	struct profilelef *cw,*ccw;//f
}VPLF,*PVPLF;//树叶
typedef struct profilelink{
	PVPLF splf,eplf;//叶子的首末点   
	int   ptype;   //whether be a single reflex or a conected closed profile;
				   //if ptype= SINGLE and reflex=1,mean the reflexmax profile;
				   //if ptype= COMMON means the profile within an closed contour;
	               //if ptype= CLOSED means the closed contour include the isle 
	               //and the whole closed contour;   
                   //in the other hand,we can only use one int for the useage of the label task,
                   //for the safe reason I use two;
}VPL,*PVPL;//一个轮廓环,树枝/也可是一棵树
///////////////////////above is the data used ,//ygm
extern __declspec(dllimport) int DrawLine(double x0,double y0,double z0,
			 double x1,double y1,double z1,int color);
//////////////////////////////////////////////////////////////////////////////ygm
PVPLF  vmallocplf();//;叶
PVCNBL vmalloccnbl();//;voronoi 节点
PVBLI  vmallocbli();//;平分线连
PVBNI  vmallocbni();//;角平分线
PVCNI  vmalloccni();//;
PVBNS  vmallocbns();//;
PVCNS  vmalloccns();//;
////////////////...........................
void   vdeletevlf(PVPL vpl);
void   vdeleteplf(PVPLF plf);
void   vdeletebli(PVBLI bli);
//////////////////...............................................
void  vdisplaybnstest(VBNS bi);
void  vdisplayplv(VPL pl);
void  vgainpointseg(PVCNS *c,PXYZ p);//;
void  vdisplaysinglebi(VBNS bi,int color);//;tested
void  vdisplayvoronoi(VCNI cnode);//;tested
/////////////////..................
void  vgainbisectorll(PVCNS l1, PVCNS l2, PVBNS *bi);//;tested unfinished
void  vgainbisectorlc(PVCNS l1, PVCNS c2, PVBNS *bi);//;tested yet unfinished
void  vgainbisectorcc(PVCNS c1, PVCNS c2, PVBNS *bi);//;tested yet unfinished
void  vgainbisector(PVCNS c1, PVCNS c2, PVBNS *bi);
/////////////////..................
void  vgainintersectlll(PVCNS l1,PVCNS l2,PVCNS l3,int *i,double t[2]);//;tested
void  vgainintersectllc(PVCNS l1,PVCNS l2,PVCNS c3,int *i,double t[2]);//;parallel untreated yet tested
void  vgainintersectlcc(PVCNS l1,PVCNS c2,PVCNS c3,int *i,double t[2]);//;testing unfinished
void  vgainintersectccc(PVCNS c1,PVCNS c2,PVCNS c3,int *i,double t[2]);//;testing unfinished
void  vgainintersect(PVCNS c1,PVCNS c2,PVCNS c3,int *i,double t[2]);
void  vgainintersectmvaluebns(PVBNS bns1,PVBNS bns2,double *t,int mblis,int * mvalue);
void  vgainintersectmvaluecns(PVBNS bns,PVCNS c1,PVCNS c2,PVCNS c3,double t,int *mvalue); 
////////////////....................................................
void vaddpointsegpre(PVCNBL cnbl);//cw 方向
void vaddpointseglast(PVCNBL cnbl);
void  vfindarcendpointo(PARC arc,PXYZ *pt,int *clockwise);
PXYZ  vgetintersectpoint(VBNS bi,double t,int m);//;finished
PXYZ  vgetintersectpointlc(VBNS bi,double t,int m);//;finished
void  vgetvalue(double a,double b,double c,double t[2],int *i);//;tested
void  vgetparaline(PLINE l,double *a,double *b,double *c);//;tested and it is o of the important function
///////////////......................................................
void  vgetllimit(PVCNS cn,double *a,double *b,double *c,double *t);//parameter line;
void  vgetcflimit(PVCNS cn,double *a,double *b,double *c,double *t);//paraeter line;
void  vgetcslimit(PVCNS cn,double *a,double *b,double *c,double *t);//parameter line;
/////////////.........................................................
void vgainarrangedcnbl(PCURVE curve,PVCNBL *cnbl,int itype);
void  vgainfistbisectorpoint(PVCNBL lcnbl,PVCNBL rcnbl);
void  vgainsinglebisector(PVCNBL cnbl,PVBLI *bt);//;
void  vchangepcurvetocnode(PCURVECMP curcmp,PVCNBL vnode);//
int   vgetlm(PVCNS l);
///////////////........................................................
int  vgetclockwisebli(PVBLI bli);
double vmultipointvector(PXYZ v1,PXYZ v2);
double vmultiplayvector(PXYZ v1,PXYZ v2);
//////////////........................
PXYZ  vgetplproject(double x,double y,PLINE line);
int   vgetposition(double x,double y,PXYZ p0,PXYZ p1);
int   vgetposition(double x,double y,double x0,double y0,double x1,double y1);//
void  vtransintovcns(PCURVE pi);
PVPL  vgaindoublelink(PCURVECMP cmp);
/////////////..............................................................
int  vgainintersectortl(PVCNBL lcnbl,PVCNS rcns,PVBNS bns,int startflag,int directionflag,PVBLI blif,double *t,int *m,PVBLI *bli);
int  vgainintersectortr(PVCNBL rcnbl,PVCNS lcns,PVBNS bns,int startflag,int directionflag,PVBLI blif,double *t,int *m,PVBLI *bli);
int  vcompareright(int m1,double t1,int m2,double t2);
int  vcomparerights(int m1,double t1, int m2,double t2, int m3,double t3, int m4,double t4,	int *mm1,double *tt1,int *mm2,double *tt2);
void vmergesingleprofile(PVPL profile);
int  vjudgeprofilesingle(PVPL profile);
void vdivideprofiletwo(PVPL profile,PVPL pl,PVPL pr);
void vgainsingleplfbli(PVPLF plf);
void vmergetwoprofile(PVPL pl,PVPL pr,PVPL *profile);
////////////................................................................
void  vchangepcurvetocnblloop(PCURVE curve,PVCNBL *cnbl);
void vgainprofilebycurve(PCURVE curve,PVPL *vpl,int itype);
void vjugedirection(PVCNBL cnbl,int *flag);
double vgainangleofvector(PXYZ v);
void  vmergecontour();
void  vdividetheprofile(PVPL i_pr,PVPL o_prl,PVPL o_prr);
PVPL  vgainvoronoi(PCOMPOSITE cmp,PCURVECMP island[],int num);
//////ygm..///////////////////////////////////////////////////////////////////////
extern __declspec(dllimport) void ReSaveProfileToolRCompPara(PPATH2D path);
extern __declspec(dllimport) int g_pkt_RComp;
extern void PreSetSpeedAndHeight(CSpeedPage *m_Page2);
extern void ModifySpeedAndHeight(CSpeedPage *m_Page2);
///1999-6-7 ygr
extern void SaveContourToParameter(NCPARA *m_para,char *name,PSELGRP hsel,PCURVECMP curvecmp);
extern void SaveRefPoint2DToParameter(NCPARA *m_para,char *name,XY point);
extern void SaveIntegerToParameter(NCPARA *m_para,char *name,int integer);
void ncCreatPathTree(unsigned long handle,NCPARA para,char *name);
///1999-6-7 ygr
void PreSetPktParameter(CNcPktSheet *spktSheet);
void ModifyPktParameter(CNcPktSheet *spktSheet);
extern PNCPARA MallocNcpara();
extern void GiveOldParaToNewPara(PNCPARA m_oldpara,PNCPARA m_newpara);
extern void SaveRefPoint3DToParameter(NCPARA *m_para,char *name,XYZ point);
extern void SaveNCWCSToParameter(NCPARA *m_para,char *name,NCWCS wc);
///////////////////////////////////////////////////////////////////////ygm
////////////////////////////////////////////////////////////////////////////
int AppNcProfile(int &step,int &flag){return 1;}
void PreSetPktParameter(CNcPktSheet *spktSheet){}
void ModifyPktParameter(CNcPktSheet *spktSheet){}
void PreSetProParameter(CNcProSheet *sproSheet){}
void ModifyProParameter(CNcProSheet *sproSheet){}
//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int AppNcPkt(int &step,int &flag)//平面区域加工
{
	static    XYZ       point;
	static    PCURVECMP pickcur=NULL;
	static    PCURVE     cur;
	static    PSELGRP   psel=NULL,sel=NULL;
	static    PCURVECMP islands=NULL,tisl=NULL;
	int					ret,num;
	double				pln_mat[4][4];
	PPATH2D				path=NULL;
	PSELGRP				seli=NULL;
	PCURVECMP			isl=NULL,*islandin=NULL;
	CNcPktSheet			pktSheet;
	static    PATH2D    temppath; //ygr 1999-6-7轨迹重算
	NCWCS     wc;
	CString sTemp; // gkq: 为了存储资源字符串
	////////////test ygm
	PVPL vpl;
	VBNS a;
	PVCNS l1,l2,l3,c1,c2,c3;
	PLINE  line1,line2,line3;
	PARC arc1,arc2,arc3;
	PVCNBL cnbl;
	double t[2];
	int i;
//	ASSERT(0);
	a.x1=56;a.x2=6;a.x3=7;a.x4=2;a.x5=50;a.x6=5;a.x7=2;a.x8=5;
	a.y1=44;a.y2=0;a.y3=80;a.y4=2;a.y5=67;a.y6=5;a.y7=4;a.y8=0;
	a.m1=1;a.m2=1;a.tmin=1;a.t1=10;a.infinite=0;a.bitype=2;
	l1=vmalloccns();l1->clockwise=CCW;l1->type=CAXLINE;l1->reflex=0;
	l2=vmalloccns();l2->clockwise=CCW;l2->type=CAXLINE;l2->reflex=0;
	l3=vmalloccns();l3->clockwise=CCW;l3->type=CAXLINE;l3->reflex=0;
	c1=vmalloccns();c1->clockwise=CW;c1->type=CAXARC;c1->reflex=0;
	c2=vmalloccns();c2->clockwise=CW;c2->type=CAXARC;c2->reflex=0;
	c3=vmalloccns();c3->clockwise=CCW;c3->type=CAXARC;c3->reflex=0;
	//bi=vmallocbns();
	line1=l1->element.line=MallocLine();
	line1->p0.x=0;line1->p0.y=-1000;line1->p1.x=0;line1->p1.y=1000;
	line2=l2->element.line=MallocLine();
	line2->p0.x=0;line2->p0.y=0;line2->p1.x=100;line2->p1.y=0;
	line3=l3->element.line=MallocLine();
	line3->p0.x=100;line3->p0.y=0;line3->p1.x=0;line3->p1.y=100;
	arc1=c1->element.arc=MallocArc();
	arc1->pc.x=0;arc1->px.x=10;arc1->py.y=10;arc1->a=3.14*2;
	arc1->pc.y=-10;
	arc1->r=5;
	arc2=c2->element.arc=MallocArc();//cw
	arc2->pc.x=10;arc2->px.x=10;arc2->px.y=0;
	arc2->pc.y=0;arc2->py.x=0;arc2->py.y=10;
	arc2->r=10;
	arc3=c3->element.arc=MallocArc();
	arc3->pc.x=0;
	arc3->pc.y=0;
	arc3->r=100;
	XYZ o;
	XYZ v;
    v.x=4;v.y=6;
//	PSTR3D o_p;
	int yyy = RGB(200,100,0);
	vgainintersect(c1,c2,l2,&i,t);
//	vdisplaybnstest(*bi);
///////////////////test
	switch(step)
	{
	case 0://填写参数表
	if(g_Mod6 != 1111)
	{
		AfxMessageBox(IDS_ILLIGIAL_MODE);
		uiEndCommand();
		return 0;
	}
		psel=NULL;
		islands=NULL;
		islandin=NULL;
		pickcur=NULL;isl=NULL;
		cla_bod=NULL;
		cla_isl=NULL;
		temppath.para.entity = NULL; //1999-6-9 ygr
		temppath.para.parameter = NULL;//1999-6-9 ygr
		path=NULL;

		uiPromptID(IDS_FILL_NCPARA);		

		AssignValueToPktCutPara();
		uiRegisterPopMenu(IDR_CONTOUR_PICK_TOOLS,0);
		//uiPrompt("拾取轮廓:");
		uiPromptID(IDS_PICK_CONTOUR);
		//vdisplaysinglebi(bi);
		
		break;
	case 1://拾取轮廓
		ret=CaxGetCloseContour(&pickcur,&psel);
		point= PickBuffer[0].pick_pt;///what is the pickBuffer?
		if(ret!=PICK_OK)
		{
			uiEndCommand();
			return 1;
		}
//// ygr 1999-6-7/// 轨迹重算
		//CaxGetNcPlaneMatrix(pln_mat);
		//SaveNcPlaneMatrixToParameter(&(temppath.para),"nc_plane_matrix",pln_mat);
		GetNCWcsFromSystemWCS(&wc);//get wc
		SaveNCWCSToParameter(&(temppath.para),"nc_working_coordinate",wc);
		SaveContourToParameter(&(temppath.para),"contour",psel,pickcur);			
		SaveRefPoint3DToParameter(&(temppath.para),"referance_point",point);
//// ygr 1999-6-7///轨迹重算
		for(sel = psel;sel->next != NULL;sel = sel->next)
			;
		step = 2;
		//uiPrompt("拾取岛:");
		uiPromptID(IDS_PICK_ISLANDS);
		uiRegisterPopMenu(IDR_ISLAND_PICK_TOOLS,0);
		break;
	case 2://拾取岛屿
		ret = CaxGetIsland(&isl,&seli);
		if(ret==PICK_ESC)
			goto finish;
		else if(ret==PICK_FSH)
			goto cal;
		else
		{
			SaveContourToParameter(&(temppath.para),"islands",seli,isl);
			if(islands == NULL)
				tisl = islands = isl;
			else
				tisl = tisl->next = isl;
			sel->next = seli;
			for(sel = seli;sel->next != NULL;sel = sel->next)
				;
		}
		break;
	}
	return 1;

cal:
	uiRegisterPopMenu();

	SaveParameterToPocketPath(&temppath);
	if(g_AutoPath==1)
	{
		PNCPARA para;

		para = NULL;
		para = MallocNcpara();
		GiveOldParaToNewPara(&(temppath.para),para);
		g_NcparaList.AddTail(para);
		goto finish;
	}
	num=0;
	islandin=NULL;
	for(tisl=islands;tisl!=NULL;tisl=tisl->next)
		num++;
	num--;
	if(num>=0)
	{
		islandin=new PCURVECMP[num+1];
		if(islandin==NULL) uiNoMemoryError();
		for(int i=0;i<=num;i++)
		{
			islandin[i]=islands;
			islands=islands->next;
		}
	}
	ptx = new PT_SECT[2*MaxPktSegNumber];
	pty = new PT_SECT[2*MaxPktSegNumber];
	if(ptx==NULL||pty==NULL) uiNoMemoryError();
	//uiPrompt("生成加工轨迹,请稍等...");
	uiPromptID(IDS_WAITING);
	CaxGetNcPlaneMatrix(pln_mat);
	cur=pickcur->cmp->curve;
	vchangepcurvetocnblloop(pickcur->cmp->curve,&cnbl);
	vgainprofilebycurve(pickcur->cmp->curve,&vpl,CCW);
	vmergesingleprofile(vpl);
	vdisplayplv(*vpl);
//	vjugedirection(cnbl,&flag,1);
//va:	 t=vgainangleofvector(&v);
//	   goto va;
//	path=pktFlwPathCreat(pickcur,islandin,num,point,pln_mat);
//	path=NULL;
	if(path==NULL)
	{
		delete[] ptx;
		delete[] pty;
		FreePathPara(temppath.para);
		goto finish;
	}
	//SaveParameterToPocketPath(path);
	path->para=temppath.para; 
	cursor_off();
	InitialUndoBuffer();
	GetNCWcsFromSystemWCS(&(path->wc));
	CaxDrawPath2D(SavePath2D(path));	
	//ncCreatPathTree(path->handle,path->para,"平面区域加工");
	sTemp.LoadString( IDS_PlanePocketCutStr );
	char str[100];
	strcpy(str, (LPCTSTR)sTemp);
	ncCreatPathTree(path->handle,path->para, str);

	uiSetModifiedFlag(TRUE);
	cursor_on();

	delete[] ptx;
	delete[] pty;
	if(islandin!=NULL)
		delete[] islandin;
	islands=NULL;
	islandin=NULL;
	pickcur=NULL;isl=NULL;
	cla_bod=NULL;
	cla_isl=NULL;
	path=NULL;
	
finish:
	ptx=pty=NULL;
	InitialSelgrp(psel);
	uiEndCommand();
	return 1;
}
/////////...................................................................
PVBNS vmallocbns()
{
	PVBNS bi=NULL;

	bi = (PVBNS)malloc(sizeof(VBNS));
	if(NULL == bi)uiNoMemoryError();

	bi->a1=bi->a2=bi->a3=bi->a4=0.0;
	bi->x1=bi->x2=bi->x3=bi->x4=bi->x5=bi->x6=bi->x7=bi->x8=0;
	bi->y1=bi->y2=bi->y3=bi->y4=bi->y5=bi->y6=bi->y7=bi->y8=0;

	bi->m1=bi->m2=1;
	bi->tmin=bi->t1=bi->t2=0;
	bi->bitype=-1;
	bi->infinite = 1;
	bi->limit =0;
	return bi;
}
PVCNS vmalloccns()
{
	PVCNS ci;

	ci = NULL;
	ci = (PVCNS)malloc(sizeof(VCNS));
	if(NULL==ci)uiNoMemoryError();
	ci->clockwise = CCW;
	ci->element.line=NULL;
	ci->reflex=0;
	ci->type=CAXLINE;
	return ci;
}
PVCNI vmalloccni()
{
	PVCNI vi;

	vi = NULL;
	vi = (PVCNI)malloc(sizeof(VCNI));
	if(NULL == vi)uiNoMemoryError();
	vi->ccw=NULL;
	vi->cw=NULL;
	vi->cns=NULL;
	return vi;
}
PVBNI vmallocbni()
{
	PVBNI bi;

	bi = NULL;
	bi = (PVBNI)malloc(sizeof(VBNI));
	if(NULL == bi)uiNoMemoryError();
	bi->bns=NULL;
	bi->selfcns=NULL;
	bi->othercns=NULL;
	bi->flag=0;
	return bi;
}
PVBLI  vmallocbli()//;平分线连
{
	PVBLI bli=NULL;

	bli=new VBLI;
	ASSERT(bli);
	bli->last=NULL;
	bli->next=NULL;
	bli->clockwise=CCW;
	bli->bni=NULL;
	return bli;
}
PVCNBL vmalloccnbl()//;voronoi 节点
{
	PVCNBL cnbl;
	cnbl=new VCNBL;
	cnbl->ccw=NULL;
	cnbl->cw=NULL;
	cnbl->flag = 0;

	cnbl->bli=NULL;
	cnbl->cns=NULL;
	return cnbl;
}
PVPL vmallocpl()
{
	PVPL  pl=NULL;
	pl=new VPL;
	ASSERT(pl);
	pl->eplf=NULL;
	pl->ptype=SINGLE;
	pl->splf=NULL;
	return pl;
}
PVPLF vmallocplf()
{
	PVPLF plf=NULL;
	plf =(PVPLF)malloc(sizeof(VPLF));
	if(NULL==plf)uiNoMemoryError();
	plf->ccw=NULL;
	plf->cw=NULL;
	plf->ecnbl=plf->scnbl=NULL;
	plf->type=SINGLEPLF;
	return plf;
}
///
void vdeletebli(PVBLI bli)
{
	if(bli!=NULL)
	{
		if(bli->bni != NULL)
			delete bli->bni;
		else
			AfxMessageBox("you have attempt to deleter NULL bni");
		delete bli;
		bli = NULL;
	}
	else{
		AfxMessageBox("you have attempt to deleter NULL bli");
	}
	return ;
}
void vdeleteplf(PVPLF plf)
{
	if(!plf)
	{
		if(plf->type==SINGLE)
		{
			vdeletebli(plf->scnbl->bli);
			delete plf->scnbl;
			delete plf;
		}
		else
		{
			vdeletebli(plf->scnbl->bli);
			delete plf->scnbl;
			vdeletebli(plf->ecnbl->bli);
			delete plf->ecnbl;
			delete plf;
		}
	}
	else
	{
		AfxMessageBox("you attempt to delete a null plf");
	}
	return;
}
void vdeletevendpointcnbl(PVCNBL cnbl)
{
	ASSERT(cnbl);
	PVBLI bli;
	PVBLI blit;
	if(cnbl->flag==1)
	{
		bli = cnbl->bli;
		while(bli!=NULL){
			blit = bli->next;
			vdeletebli(bli);
			bli = blit;
		}
		delete cnbl->cns;
		delete cnbl;
		cnbl = NULL;
	}
	else 
	{
		AfxMessageBox("please check the data before you delete the cnbl");
	}
	return;
}
/////.................................................................
void  vdisplaybnstest(VBNS bi)
{
	PLINE line;
	double x1,x2,x3,x4,x5,x6,x7,x8,m1;
	double y1,y2,y3,y4,y5,y6,y7,y8,m2;
	double tmax,t1,t2;
	double tt,t0;
	double tm;
	XYZ p0,p1,p2,p3,p4;
	x1=bi.x1;x2=bi.x2;x3=bi.x3;x4=bi.x4;x5=bi.x5;x6=bi.x6;x7=bi.x7;x8=bi.x8;
	y1=bi.y1;y2=bi.y2;y3=bi.y3;y4=bi.y4;y5=bi.y5;y6=bi.y6;y7=bi.y7;y8=bi.y8;
	t1 = bi.t1;m1 = bi.m1;t0 = bi.tmin;
	t2 = bi.t2;m2 = bi.m2;tmax = bi.tmax;
	line = MallocLine();
			p1.x = p0.x = 0;
			p1.y = p0.y = 0;
			p1.z = p0.z = 0;
			int fff = 0;
			for(tm = -UPER;tm<UPER;tm+=STEP)
			{
				t0 = tm+STEP;
				tt = pow(x5+x6*(t0),2)-pow(x7+x8*(t0),2);
				if(tt>=0)
				{
					if(fff==0)
					{
					p0.x=x1-x2-x3*(t0)+x4*sqrt(tt);
					p0.y=y1-y2-y3*(t0)-y4*sqrt(tt);
					p1.x=x1-x2-x3*(t0)-x4*sqrt(tt);
					p1.y=y1-y2-y3*(t0)+y4*sqrt(tt);
					fff=1;
					continue;
					}
					p3.x=x1-x2-x3*(t0)+x4*sqrt(tt);
					p3.y=y1-y2-y3*(t0)-y4*sqrt(tt);
					p4.x=x1-x2-x3*(t0)-x4*sqrt(tt);
					p4.y=y1-y2-y3*(t0)+y4*sqrt(tt);
				}
				else continue;
				p3.z = 0.0;
				p4.z = 0.0;
				line->p0 = p0;
				line->p1 = p3;
				p0=p3;
				SaveLine(line);
				CaxDrawLine(line);
				line=NULL;
				line= MallocLine();
				////
				line->p0 = p1;
				line->p1 = p4;
				p1 = p4;
				SaveLine(line);
				CaxDrawLine(line);
				line=NULL;
				line= MallocLine();
			}
	return;
}
void  vdisplaysinglebi(VBNS bi,int color)
{
	PCURVECMP cur=NULL;
	PLINE line;
	double x1,x2,x3,x4,x5,x6,x7,x8,m1;
	double y1,y2,y3,y4,y5,y6,y7,y8,m2;
	double tmax,t1,t2,tmin;
	double tm,tstep;
	double detax,detay,detax0,detay0;
	XYZ p0,p1;
	x1=bi.x1;x2=bi.x2;x3=bi.x3;x4=bi.x4;x5=bi.x5;x6=bi.x6;x7=bi.x7;x8=bi.x8;
	y1=bi.y1;y2=bi.y2;y3=bi.y3;y4=bi.y4;y5=bi.y5;y6=bi.y6;y7=bi.y7;y8=bi.y8;
	t1 = bi.t1;m1 = bi.m1;tmin = bi.tmin;
	t2 = bi.t2;m2 = bi.m2;tmax = bi.tmax;
	line = MallocLine();
	switch (bi.bitype)
	{
	case LL:
	    p1.x = x1-x3*t2;
		p1.y = y1-y3*t2;
		p0.x = x1-x3*t1;
        p0.y = y1-y3*t1;
		p0.z = 0.0;
		p1.z = 0.0;
		line->p0 = p0;
		line->p1 = p1;
		//CaxDrawLine(line);
				cur=SaveLine(line);
				cur->color = color;
				CaxDrawCurvecmp(cur);
				cur = NULL;
				//SaveLine(line);
		break;
	case CC:
	case LC://直线与圆弧求交
		if(m1 == m2)
		{
			tstep = m1*(t2-t1)/6;
			p0.x=x1-x2-x3*t1+m1*x4*sqrt(pow(x5+x6*t1,2)-pow(x7+x8*t1,2));
			p0.y=y1-y2-y3*t1-m1*y4*sqrt(pow(y5+y6*t1,2)-pow(y7+y8*t1,2));
			p0.z = 0.0;
			for(tm = t1*m1; t2*m2-tm>=ZERO;tm+=tstep)
			{
				p1.x=x1-x2-x3*(tm+tstep)*m1+m1*x4*sqrt(pow(x5+x6*(tm+tstep)*m1,2)-pow(x7+x8*(tm+tstep)*m1,2));
				p1.y=y1-y2-y3*(tm+tstep)*m1-m1*y4*sqrt(pow(y5+y6*(tm+tstep)*m1,2)-pow(y7+y8*(tm+tstep)*m1,2));
				p1.z = 0.0;
				line->p0 = p0;
				line->p1 = p1;
				p0 = p1;
				cur=SaveLine(line);
				cur->color = color;
				CaxDrawCurvecmp(cur);
				cur = NULL;
				//CaxDrawLine(line);
				line=NULL;
				line= MallocLine();
			}
		}
		else if(m1 != m2)
		{
			tstep = (t1-tmin)/3;
			detax0 = pow(x5+x6*tmin,2)-pow(x7+x8*tmin,2);
			if(detax0 <= ZERO)
				detax0 = 0;
			else detax0 = sqrt(detax0);
			detay0 = pow(y5+y6*tmin,2)-pow(y7+y8*tmin,2);
			if(detay0 <= ZERO)
				detay0 = 0;
			else detay0 = sqrt(detay0);
			p0.x=x1-x2-x3*tmin+x4*detax0;
			p0.y=y1-y2-y3*tmin-y4*detay0;
			for(tm = tmin;t1-tm>ZERO;tm+=tstep)
			{
				detax = pow(x5+x6*(tm+tstep),2)-pow(x7+x8*(tm+tstep),2);
				if(detax <= ZERO)
					detax = 0;
				else detax = sqrt(detax);
				detay = pow(x5+x6*(tm+tstep),2)-pow(x7+x8*(tm+tstep),2);
				if(detay <= ZERO)
					detay = 0;
				else detay = sqrt(detay);
				p1.x=x1-x2-x3*(tm+STEP)+m1*x4*detax;
				p1.y=y1-y2-y3*(tm+STEP)-m1*y4*detay;
				line->p0 = p0;
				line->p1 = p1;
				p0 = p1;
					//
					cur = SaveLine(line);
					cur->color = color;
					CaxDrawCurvecmp(cur);
					cur = NULL;
					//
				line=NULL;
				line= MallocLine();
			}
			tstep = (t2-tmin)/3;
			p0.x=x1-x2-x3*tmin+x4*detax0;
			p0.y=y1-y2-y3*tmin-y4*detay0;
			for(tm = tmin;t2-tm>ZERO;tm+=tstep)
			{
				detax = pow(x5+x6*(tm+tstep),2)-pow(x7+x8*(tm+tstep),2);
				if(detax <= ZERO)
					detax = 0;
				else detax = sqrt(detax);
				detay = pow(x5+x6*(tm+tstep),2)-pow(x7+x8*(tm+tstep),2);
				if(detay <= ZERO)
					detay = 0;
				else					
					detay = sqrt(detay);
				p1.x=x1-x2-x3*(tm+tstep)+m2*x4*detax;
				p1.y=y1-y2-y3*(tm+tstep)-m2*y4*detay;
				line->p0 = p0;
				line->p1 = p1;
				p0 = p1;
					//
					cur = SaveLine(line);
					cur->color = color;
					CaxDrawCurvecmp(cur);
					cur = NULL;
					//
				line=NULL;
				line= MallocLine();
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}
	return;
}
void  vdisplayvoronoi(VCNBL cn)
{
	PVBNS  bi=NULL;
	PVBLI  bil=NULL;
	PVCNBL tmnode=NULL;
	tmnode = &cn;
	int color=RGB(255,0,0);
	int colorstep = 20;
	do{
		color += colorstep;
		bil = tmnode->bli;
		do{
			bi = bil->bni->bns;
			vdisplaysinglebi(*bi,color);
			bil = bil->last;
		}while(bil);
	tmnode = tmnode->ccw;
	}while(tmnode!=&cn);
	return;
}
void vdisplayplv(VPL pl)
{
	PVCNBL cnbls,cnble,cnbl;
	PVBLI  bli;
	PVBNS  bns;
	cnbl = cnbls = pl.splf->scnbl;
	cnble = pl.eplf->ecnbl;
	int colorstep = 20;
	while(cnbl!=NULL&&cnbl!=cnble)
	{
		bli = cnbl->bli;
		while(bli!=NULL)
		{
			if(bli->bni->flag==1);
			else{
				bns = bli->bni->bns;
				vdisplaysinglebi(*bns,RGB(200,0,200)+myflag*colorstep);
				//AfxMessageBox("2");
				bli->bni->flag =1;
				bli=bli->next;
			}
		}
		cnbl = cnbl->ccw;
	}
	if(cnbl==cnble)
	{
		bli = cnbl->bli;
		while(bli!=NULL)
		{
			if(bli->bni->flag==1);
			else{
				bns = bli->bni->bns;
				vdisplaysinglebi(*bns,RGB(200,0,200));
				bli->bni->flag =1;
				bli=bli->next;
			}
		}
	}
	cnbl = pl.splf->scnbl;
	do
	{
		bli = cnbl->bli;
		while(bli!=NULL)
		{
			bli->bni->flag =0;
			bli=bli->next;
		}
		cnbl = cnbl->ccw;
	}while(cnbl!=NULL&&cnbl!=cnble);
	if(cnbl!=NULL)
	{
		bli = cnbl->bli;
		while(bli!=NULL)
		{
			bli->bni->flag =0;
			bli=bli->next;
		}
		cnbl = cnbl->ccw;
	}
}
//////////........................................................
//first we assume that the t1,t2 >=0;
//if return 1 means the first is to the left of t2,m1t1 < m2t2
// 2,vias
//0 means m1t1==m2t2;
int vcompareright(int m1,double t1,int m2,double t2)
{
	double tm1,tm2;
	tm1 = m1*t1;
	tm2 = m2*t2;
	if(fabs(tm1-tm2)<=ZERO)
		return 0;
	else if(tm1-tm2>ZERO)
		return 2;
	else if(tm1-tm2<-ZERO)
		return 1;
}
//find the value of the line direction
void vfindvectorinpositive(PVBNS bns,int m,double t,PXYZ *p)
{//as for the CCP I assume that the p is head for alph adding direction
	ASSERT(bns);
	PXYZ pp = NULL;
	double x1,x0,y1,y0,deta,detax,detay;
	if(LLP == bns->bitype)
	{
		pp = new XYZ;
		if(m>0){
			pp->x =  bns->x2;
			pp->x = -bns->x1;
		}
		else{
			pp->x = -bns->x2;
			pp->y =  bns->x1;
		}
		*p = pp;
	}
	else if (CCP == bns->bitype)
	{
		pp = new XYZ;
		pp->x = cos(t*m + PI/2);
		pp->y = sin(t*m + PI/2);
		*p = pp;
	}
	else if (LL == bns->bitype)
	{
		pp = new XYZ;
		pp->x =  -bns->x3;
		pp->y =  -bns->y3;
		pp->z =   0;
		*p = pp;
	}
	else if (LC == bns->bitype || CC == bns->bitype)
	{
		deta = pow(bns->x5 + bns->x6*t,2) - pow(bns->x7 + bns->x8*t,2);
		if(deta<-ZERO)ASSERT(0);
		if(fabs(deta)<=ZERO){
			x0 = -bns->x3; x1 = 0;
			y0 = -bns->y3; y1 = 0;		
		}
		else
		{
			detax = m*bns->x4/sqrt(deta);
			x0 = detax*(bns->x5*bns->x6 - bns->x7*bns->x8) - bns->x3;
			x1 =  detax*(bns->x6*bns->x6 - bns->x8*bns->x8);
			deta = pow(bns->y5 + bns->y6*t,2) - pow(bns->y7 + bns->y8*t,2);//can be deleted safely
			detay = -m*bns->y4/sqrt(deta);
			y0 = detay*(bns->y5*bns->y6 - bns->y7*bns->y8) - bns->y3;
			y1 = detay*(bns->y6*bns->y6 - bns->y8*bns->y8);//please pay attention
		}
		pp = new XYZ;
		pp->x = x0 + t*x1;
		pp->y = y0 + t*y1;
		pp->z = 0;
		*p = pp;
	}
}
//I do not know whether this function if ture
//m is the m of bns
int vfinddirection(PVBLI bli,PVBNS bns,double t,int m)
{//t and m is the intersect point of between bli->bni->bns and bns
	ASSERT(bns);
	PXYZ  pa,pb;
	int mm;//mm is the m of bli->bni->bns; 
	double tm;
	double tt = t;
	vfindvectorinpositive(bns,m,tt,&pa);
	//
	vgainintersectmvaluebns(bli->bni->bns,bns,&tt,m,&mm);
	vfindvectorinpositive(bli->bni->bns,mm,tt,&pb);//we know that we must find all the information of bli
	tm = vmultiplayvector(pb,pa);
	//
	tm = tm*m*mm*bli->clockwise;//this if the critical judgement
	delete pa;
	delete pb;
	if(tm>=0)//==0 means parallel if so ,there will no intersect in funture caculation 
	{
		return CCW;//this mean the t*m >tfirst*mfirst is validative
	}
	else
	{
		return CW;//otherwise mean the t*m behand the tfirst*mfirst if validative
	}
}
//find the overlaps of m1t1-m2t2 and m3r3-m4t4;
//if return 0,no overlaps;
//if return 1,only one point overlaps;
//if return 2,have segemented overlaps;
int  vcomparerights(int m1,double t1, int m2,double t2, //input
					int m3,double t3, int m4,double t4, //input
					int *mm1,double *tt1,
					int *mm2,double *tt2)
{//I think it will be ok.
	int m;
	double t;
	double tm1,tm2,tm3,tm4;
	tm1 = m1*t1; tm2 = m2*t2; tm3 = m3*t3;tm4 = m4*t4;
	if(tm1-tm2>ZERO)
	{
		m=m1; t=t1;
		m1=m2; t1=t2;
		m2=m; t2=t;
	}
	if(tm3-tm4>ZERO)
	{
		m=m3; t=t3;
		m3=m4; t3=t4;
		m4=m; t4=t;
	}
	//
	if((tm1 >m4*t4)||(tm3 > tm2))
		return 0;
	//
	if(fabs(tm1-tm2)<ZERO)
	{
		if(tm1>=tm3&&tm1<=tm4)
		{
			*mm1=*mm2=m1;
			*tt1=*tt2=t1;
			return 1;
		}	   	
		else return 0;
	}
	if(fabs(tm3-tm4)<ZERO)
	{
		if(tm3>=tm1&&tm3<=tm2)
		{
			*mm1=*mm2=m3;
			*tt1=*tt2=t3;
			return 1;
		}	   	
		else return 0;
	}
	//
	if(tm1 >= tm3 && tm1 <= tm4)
	{
		*tt1 = t1; *mm1 = m1;
		if(m2*t2 >= m4*t4)
		{
			*tt2 = t4;*mm2= m4;
		}
		else if(m2*t2 < m4*t4)
		{
			*tt2 = t2;*mm2= m2;
		}
		return 2;
	}
	else if(tm1 < tm3)
	{
		*tt1=t3; *mm1=m3;
		if(tm2 >= tm4)
		{
			*tt2 = t4;*mm2= m4;
		}
		else if(tm2 >= tm3)
		{
			*tt2 = t2;*mm2= m2;
		}
		return 2;
	}
}
void  vgetparaline(PLINE l,double *a,double *b,double *c)
{
	ASSERT(l);
	double x0,y0,x1,y1;
	double delt;
	x0 = l->p0.x; y0 = l->p0.y;
	x1 = l->p1.x; y1 = l->p1.y;
	if(fabs(x1-x0)<ZERO)*b=0;else *b=x1-x0;
	if(fabs(y1-y0)<ZERO)*a=0;else *a=y0-y1;
	ASSERT(0!=*a||0!=*b);
	delt = sqrt(pow(*a,2)+pow(*b,2));
	*a = *a/delt;*b = *b/delt;
	*c = ((y1-y0)*x0-y0*(x1-x0))/delt;
	return;
}
PXYZ  vgetintersectpoint(VBNS bi,double t,int m)
{
	PXYZ p=new XYZ;
	if(bi.bitype==LL)
	{
		p->x = bi.x1-bi.x2-bi.x3*t;
        p->y = bi.y1-bi.y2-bi.y3*t;
		p->z = 0.0;
	}
	else
	{
		p->x=bi.x1-bi.x2-bi.x3*t+m*bi.x4*sqrt(pow(bi.x5+bi.x6*t,2)-pow(bi.x7+bi.x8*t,2));
		p->y=bi.y1-bi.y2-bi.y3*t-m*bi.y4*sqrt(pow(bi.y5+bi.y6*t,2)-pow(bi.y7+bi.y8*t,2));
		p->z=0;
	}
	return p;
}
//find resultion of ax2+bx+c=0;
void  vgetvalue(double a,double b,double c,double t[2],int *i)
{
	double deta;
	if(fabs(a)<ZERO){
		ASSERT(fabs(b)>ZERO);
		if(fabs(b)<ZERO){
			*i=0;
		}
		else{
			t[0]=-c/b;
			*i=1;
		}
	}
	else if(fabs(b)<ZERO)
	{
		ASSERT(fabs(a)>ZERO);
        if(fabs(c)<ZERO){
			*i=1;
			t[0]=0;
		}
		else if(c<-ZERO){
			*i=0;
		}
		else {
			*i=2;
			t[0]=sqrt(c);
			t[1]=-sqrt(c);
		}
	}
	else
	{
		deta=b*b-4*a*c;
		if(deta<-ZERO){
			*i=0;
			t[0]=0;
		}
		else if(fabs(deta)<ZERO)
		{
			*i=1;
			t[0]=-b/2/a;
		}
		else 
		{
			*i=2;
			t[0]=(-b+sqrt(deta))/2/a;
			t[1]=(-b-sqrt(deta))/2/a;
		}
	}
	return;
}
//find the  validate areas;
void vgettminandtmaxvalue( double a,double b,double c,double t[4],int *i )
{//i=0 ,mean no area,1 mean 1area,2 mean,two area
	double deta,o,p;
	double tt[2];
	if(fabs(a)<=ZERO)
	{
		if(fabs(b)<=ZERO){
			if(c>=-ZERO){*i = 1;t[0] = 0;t[1] = UPER;}
			else *i = 0;
		}
		else if( b<-ZERO )
		{
			if(fabs(-c/b)<=0) *i = 0;
			else{
				*i = 1;
				t[0] = 0;t[1] = -c/b;
			}
		}
		else
		{
			*i = 1;
			t[1] = UPER;
			if(-c/b>=0)
				t[0] = -c/b;
			else t[0] =0;
			return;
		}
	}
	else{
		deta = b*b-4*a*c;
		if(a>ZERO)
		{
			if(deta<=0){*i=1;t[0]=-UPER;t[1] = UPER;}
			else{
				o = (-b-sqrt(deta))/(2*a);p = (-b+sqrt(deta))/(2*a);
				if(o>=0){*i = 2;t[0]=0;t[1] = o;t[2]=p,t[3] = UPER;}
				else if(p>=0){*i=1;t[0] = p;t[1] =UPER;}
				else if(p<0) {*i=1;t[0] = 0;t[1] =UPER;}
			}
		}
		else if(a<-ZERO)
		{
			if(deta<=0){*i=0;}
			else{
				o = (-b-sqrt(deta))/(2*a);p = (-b+sqrt(deta))/(2*a);
				if(o>=0){*i = 1;t[0]=o;t[1] = p;}
				else if(o<0&&p>=0){*i=1;t[0] = 0;t[1] = p;}
				else if(p<0) {*i=0;}
			}
		}
	}
	return;
}
//gain a b c&t ;
void  vgetllimit(PVCNS cn,double *a,double *b,double *c,double *t)
{
	ASSERT(cn);
	ASSERT(cn->type==0);
	double x0,y0,x1,y1;
	double delt;
	XYZ p0,p1;
	if(0==cn->clockwise){
		p0 = cn->element.line->p0;
		p1 = cn->element.line->p1;
	}
	else if(1==cn->clockwise){
		p1 = cn->element.line->p0;
		p0 = cn->element.line->p1;
	}
	x0 = p0.x;y0 = p0.y;
	x1 = p1.x;y1 = p1.y;
	if(fabs(x1-x0)<ZERO)*a=0;else *a=x1-x0;
	if(fabs(y1-y0)<ZERO)*b=0;else *b=y1-y0;
	ASSERT(0!=*a&&0!=*b);
	delt = sqrt(pow(*a,2)+pow(*b,2));
	*a = *a/delt;
	*b = *b/delt;
	*c = -((y1-y0)*y0+x0*(x1-x0))/delt;
	*t = 0;
	return;
}
void  vgetcflimit(PVCNS cn,double *a,double *b,double *c,double *t)
{
	ASSERT(cn);
	ASSERT(cn->element.arc);
	PLINE line;
//	XYZ pt,du,duu,dn;
//	GeneratorArcAllValue(cn->element.arc,0.5,&pt,&du,&duu,&dn);
	line->p0=cn->element.arc->pc;
	line->p1=cn->element.arc->px;
	vgetparaline(line,a,b,c);
	*t = cn->element.arc->r;
	return;
}
void  vgetcslimit(PVCNS cn,double *a,double *b,double *c,double *t)
{
	ASSERT(cn);
	ASSERT(cn->element.arc);
	PLINE line;
	line->p0=cn->element.arc->pc;
	line->p1=cn->element.arc->px;
	vgetparaline(line,a,b,c);
	*t = cn->element.arc->r;
	return;
}
//
void  vgainbisectorll(PVCNS l1, PVCNS l2, PVBNS *bi)//ok
{
	ASSERT(l1);ASSERT(l2);ASSERT(CAXLINE==l1->type);ASSERT(CAXLINE==l2->type);
	PVBNS  bt = NULL;
	*bi = NULL;
	XYZ p0,p1;
	double a1,b1,c1;
	double a2,b2,c2;
	double deta;
	int    m1,m2;
	if(CCW == l1->clockwise)m1 = -1;
	else m1 = 1;
	if(CCW == l2->clockwise)m2 = -1;
	else m2 = 1;
	vgetparaline(l1->element.line,&a1,&b1,&c1);
	vgetparaline(l2->element.line,&a2,&b2,&c2);
	deta = a1*b2 - b1*a2;
	if(fabs(deta)<=ZERO)//处理平行线
	{//Anyway the same line have bisector too; 
		ASSERT(0);//kill it afterwards
		double tmtemp;
		if(m1 == m2) return;
		tmtemp = (c2-c1)/(m1-m2);
		if(tmtemp<-ZERO) return;//we must be sure of the positive of integret
		bt = vmallocbns();
		bt->bitype = LLP;
		bt->tmin = bt->tmax = tmtemp;
		bt->x1 = a1;
		bt->x2 = b1;
		bt->x3 = c1 + m1*bt->tmin;//一般方程a**2+b**2 = 1;
		bt->x4 = l1->element.line->p0.x + a1 * m1*tmtemp;
		bt->y4 = l1->element.line->p0.y + b1 * m1*tmtemp;//find the first point
		*bi = bt;
		//以下对t1,t2赋值
		return;
	}//below 为平分线赋值
	bt = vmallocbns();
    bt->x1 = (b1*c2-b2*c1)/deta;
    bt->x3 =-(b1*m2-b2*m1)/deta;
    bt->y1 = (a2*c1-a1*c2)/deta;
    bt->y3 =-(a2*m1-a1*m2)/deta;
	bt->bitype = LL;
	bt->infinite = INFINITY;
	//以下对t1,t2赋值
	bt->t1 = 0;
	bt->t2 = UPER;
	bt->m1=1;
	bt->m2=1;
	*bi = bt;
	return ;
}
void  vgainbisectorlc(PVCNS l1, PVCNS cc2, PVBNS *bi)
{
	ASSERT(l1);ASSERT(cc2);ASSERT(CAXARC==cc2->type);ASSERT(CAXLINE==l1->type);
	double xc1,yc1,r1,a2,b2,c2;
	double h,m1,m2;
	double aa,bb,cc,tt[4];
	int    iflag;
	vgetparaline(l1->element.line,&a2,&b2,&c2);
	if(CCW==cc2->clockwise)m1 = 1;
	else m1 = -1;
	if(CCW==l1->clockwise)m2 = -1;
	else m2 = 1;
//	GeneratorArcAllValue(c2->element.arc,1.0,&pt,&du,&duu,&dn);
	xc1=cc2->element.arc->pc.x;
	yc1=cc2->element.arc->pc.y;
	r1 =cc2->element.arc->r;

	h = a2*xc1 + b2*yc1 + c2; 
	PVBNS bt = vmallocbns();
    //x1-x2-x3*t+x4*sqrt((x5+x6*t)**2-(x7+x8*t)**2);
	bt->x1=xc1;   bt->x2=a2*h;  bt->x3=m2*a2;   bt->x4=b2;  bt->x5=r1;
	bt->x6=m1;    bt->x7=h;     bt->x8=m2;
	bt->y1=yc1;   bt->y2=b2*h;  bt->y3=m2*b2;   bt->y4=a2;  bt->y5=r1;
	bt->y6=m1;    bt->y7=h;     bt->y8=m2;	

	bt->m1 = -1;
	bt->m2 = 1;
	//left out for later processing
/*	aa = bt->x6*bt->x6 - bt->x8*bt->x8;
	bb = 2*(bt->x5*bt->x6 - bt->x7*bt->x8);
	cc = bt->x5*bt->x5 - bt->x7*bt->x7;
	vgettminandtmaxvalue(aa,bb,cc,tt,&iflag);
	if(iflag == 1)*/
	//////////assign tmin;
	aa = h*h-r1*r1;//t(2r1m1-2hm2)>=h**2-r1**2;
	bb = 2*(r1*m1-h*m2);
	if(fabs(bb)<=ZERO)
	{
		if(aa<=ZERO)bt->tmin = 0;
		else ASSERT(0);
	}
	else if(bb<-ZERO)
	{
		if(aa/bb>ZERO){bt->tmin = 0;bt->tmax = aa/bb;}
		else ASSERT(0);
	}
	else if(bb>ZERO)
	{
		if(aa<=ZERO)
			bt->tmin = 0;
		else {
			bt->tmin = aa/bb ; 
		}
	}
	//上面可以不要
	bt->t1 = UPER;
	bt->t2 = UPER;
	bt->m1 = -1;
	bt->m2 =  1;
	bt->bitype = LC;
	bt->infinite = INFINITY;
	*bi = bt;
	return;
}
void  vgainbisectorcc(PVCNS c1, PVCNS c2, PVBNS *bi)
{
	ASSERT(c1);ASSERT(c2);ASSERT(CAXARC==c1->type);ASSERT(CAXARC==c2->type);
	double xc1,yc1,r1,xc2,yc2,r2;
	double h,m1,m2;
	double deta,dx,dy,d;

	xc1=c1->element.arc->pc.x;  xc2=c2->element.arc->pc.x;
	yc1=c1->element.arc->pc.y;  yc2=c2->element.arc->pc.y;
	r1 =c1->element.arc->r;     r2 =c2->element.arc->r;
	if(CCW==c1->clockwise)m1 = 1;
	else m1 = -1;
	if(CCW==c2->clockwise)m2 = 1;
	else m2 = -1;
//	ASSERT(pow(xc1-xc2)+pow(yc1-yc2)>ZERO);
	if(pow(xc1-xc2,2) + pow(yc1-yc2,2)<=ZERO)//同心处理
	{
		ASSERT(0);
		double t;
		if(m1 == m2)
		{		
			t = -(r1+r2)/(m2+m1);
			if(t<-ZERO)
			{
				*bi = NULL;
				return;
			}
		}
		else if(fabs(m1+m2)<=ZERO)
		{
			t = (r1-r2)/(m2-m1);
			if(t<-ZERO){
				*bi = NULL;
				return;
			}
		}
		PVBNS bt = vmallocbns();
		bt->bitype = CCP;//标识同心
		bt->x1 = (xc1 - xc2)/2;//圆心
		bt->y1 = (yc1 - yc2)/2;//圆心
		bt->x2 = m1*t + r1;//半径
		bt->infinite = 1;
		*bi = bt;
		return;
	}//below non同心处理
//	if(m1*m2<0)
//	{
//	}//圆包含处理#
	PVBNS bt = vmallocbns();
	//
	d=sqrt(pow(xc1-xc2,2)+pow(yc1-yc2,2));
	dx=(xc2-xc1)/d;dy=(yc2-yc1)/d;
	deta=(m2*r2-m1*r1)/d;
	h=(r2*r2-r1*r1-d*d)/2/d;
	//
	bt->x1=xc1;  bt->x2=dx*h;  bt->x3=dx*deta;  bt->x4=dy;  bt->x5=r1;
	bt->x6=m1;   bt->x7=h;     bt->x8=(r2*m2-r1*m1)/d;
	bt->y1=yc1;  bt->y2=dy*h;  bt->y3=dy*deta;  bt->y4=dx;  bt->y5=r1;
	bt->y6=m1;   bt->y7=h;     bt->y8=(r2*m2-r1*m1)/d;
	
    //	bt->t0 = (d-r1*m1-r2*m2)/2;
	//assign the tmin and the tmax;
	deta = r1*m1-h*bt->y8;
	if(fabs(deta)<ZERO)
	{
		bt->tmin = 0;
	}
	else if(deta<-ZERO)
	{
		bt->tmin=(r1*r1-bt->x8*bt->x8)/2/(bt->x5*bt->x6-bt->x7*bt->x8);
		bt->limit=1;
	}
	else 
	{
		bt->tmin=(r1*r1-bt->x8*bt->x8)/2/(bt->x5*bt->x6-bt->x7*bt->x8);
		bt->limit=0;
	}
	bt->tmin = 0;
	bt->tmax = UPER;
	//不要不要了，酸什么TMIN和TMAX了，去掉去掉
	bt->bitype = CC;
	bt->infinite = INFINITY;
	bt->t1 = UPER;
	bt->t2 = UPER;
	bt->m1 = -1;
	bt->m2 =  1;
	*bi=bt;
	return;
}
void  vgainbisector(PVCNS c1, PVCNS c2, PVBNS *bi)
{
	PVBNS bt;
	if(CAXLINE==c1->type)
	{
		if(CAXLINE==c2->type)
			vgainbisectorll(c1,c2,&bt);
		else if(CAXARC==c2->type)
			vgainbisectorlc(c1,c2,&bt);
	}
	else if(CAXARC==c1->type)
	{
		if(CAXLINE==c2->type)
			vgainbisectorlc(c2,c1,&bt);
		else if(CAXARC==c2->type)
			vgainbisectorcc(c1,c2,&bt);
	}
	*bi = bt;
}
//gain the the point of line line line intersect
//bisector l1&l2 intersect bisector l1&l3
//it have been ok but need text for further use
void vgainintersectlll(PVCNS l1,PVCNS l2,PVCNS l3,int *i,double t[2])
{
	ASSERT(l1);ASSERT(l2);ASSERT(l3);
	ASSERT(CAXLINE==l1->type);
	ASSERT(CAXLINE==l2->type);ASSERT(CAXLINE==l3->type);
	double a1,b1,c1,a2,b2,c2,a3,b3,c3;
	double x0,y0;
	double deta;
	int m1,m2,m3;
	vgetparaline(l1->element.line,&a1,&b1,&c1);
	vgetparaline(l2->element.line,&a2,&b2,&c2);
	vgetparaline(l3->element.line,&a3,&b3,&c3);
	if(CCW == l1->clockwise)m1 = -1;
	else m1 = 1;
	if(CCW == l2->clockwise)m2 = -1;
	else m2 = 1;
	if(CCW == l3->clockwise)m3 = -1;
	else m3 = 1;
	deta = (m1*b1-m3*b3)*(a1*m1-a2*m2)-(b1*m1-b2*m2)*(a1*m1-a3*m3);
	if(fabs(deta)<ZERO)//two line is parallel;//no it is two bisector parallel
	{
		ASSERT(0);    //if have only two line parallel,deta will not be zero 
		*i=0;		  //there are not intersect points
		t[0] = UPER;
		return;       //to be simple it is no intersector
	}
	x0 = (b1*m2-b2*m1)*(c1*m3-c3*m1) - (b1*m3-b3*m1)*(c1*m2-c2*m1);
	y0 = (a1*m2-a2*m1)*(c1*m3-c3*m1) - (a1*m3-a3*m1)*(c1*m2-c2*m1);
	x0 =  x0/deta;
	y0 = -y0/deta;
	t[0] = m1*(-a1*x0-b1*y0-c1);
	*i = 1;
    return;			
}
//i=0,1,or 2;
void vgainintersectllc(PVCNS l1,PVCNS l2,PVCNS c3,int *i,double t[2])
{
	ASSERT(l1);ASSERT(l2);ASSERT(c3);
	ASSERT(CAXLINE==l1->type);
	ASSERT(CAXLINE==l2->type);ASSERT(CAXARC==c3->type);	
	//
	double a1,b1,c1,a2,b2,c2,xc3,yc3,r3;
	double a,b,c,deta,x0,x1,y0,y1;
	int m1,m2,m3;
	PVBNS bns = NULL;
	double dtemp;
	//double tt[2];//template uesed
	
	vgetparaline(l1->element.line,&a1,&b1,&c1);
	vgetparaline(l2->element.line,&a2,&b2,&c2);
	xc3=c3->element.arc->pc.x;
	yc3=c3->element.arc->pc.y;
	r3=c3->element.arc->r;
	if(CCW==l1->clockwise)m1 = -1;
	else m1 = 1;
	if(CCW==l2->clockwise)m2 = -1;
	else m2 = 1;
	if(CCW==c3->clockwise)m3 = 1;
	else m3 = -1;
	deta = a1*b2-b1*a2;
	if(fabs(deta)<=ZERO)// two parallel  line left for later write
	{
		double ta;
		if(m1 == m2)
		{
			*i = 0;
			return;
		}
		//
		ta = (c2-c1)/(m1-m2);
		//t is minor than zero is unbearable so kick it out without mercy
		if(ta<=-ZERO){
			*i = 0;
			return;
		}
		else
		{
			vgainbisectorlc(l1,c3,&bns);
			dtemp = (bns->x5 + bns->x6*ta)*(bns->x5 + bns->x6*ta) -
				(bns->x7 + bns->x8*ta)*(bns->x7 + bns->x8*ta);
			delete bns; bns = NULL;
			if(dtemp<-ZERO){
				*i=0;
			}
			else{
				*i = 1;
				t[0] = t[1] = ta;
			}
			return;//if
		}
		//
/*		tt = a1*xc3 + b1*yc3 + (c1+m1*ta);//tt is the distance between the center of circle and the line.
		if(fabs(tt)-(r3+m3*ta)<=ZERO)
		{
			*i = 1;
			t[0] = ta;
		}
		else if(fabs(tt)-(r3+m3*ta)>ZERO)
		{
			*i = 0;
		}
		//ASSERT(0);
		AfxMessageBox("sorry but I find two duplicit lines");
		return;   */
	}
	x0 = (b1*c2-b2*c1)/deta;
	x1 = (m2*b1-b2*m1)/deta;
	y0 =-(a1*c2-a2*c1)/deta;
	y1 =-(a1*m2-a2*m1)/deta;
	 
	a=x1*x1+y1*y1-1;
	b=2*(x1*(x0-xc3)+y1*(y0-yc3)-r3*m3);
	c=(x0-xc3)*(x0-xc3)+(y0-yc3)*(y0-yc3)-r3*r3;

	vgetvalue(a,b,c,t,i);
	if(*i == 1 && t[0]<-ZERO)
	{
		*i = 0 ;
	}
	if(*i == 2)
	{
		if(t[0]<-ZERO&&t[1]<-ZERO){
			*i = 0;
		}
		else if(t[0]>=-ZERO&&t[1]<-ZERO)
		{
			*i = 1;
		}
		else if (t[0]<-ZERO&&t[1]>=-ZERO)
		{
			*i = 1;
			t[0] = t[1];
		}
	}
	return ;
}
//i=0,1,or 2;
void  vgainintersectlcc(PVCNS l1,PVCNS cc2,PVCNS c3,int *i,double t[2])//
{
	ASSERT(l1);ASSERT(cc2);ASSERT(c3);
	ASSERT(CAXLINE==l1->type);
	ASSERT(CAXARC==cc2->type);ASSERT(CAXARC==c3->type);
	
	//
	double a1,b1,c1,xc2,yc2,r2,xc3,yc3,r3;
	double a2,b2,c2,m;
	double a,b,c,deta,x0,x1,y0,y1;
	int m1,m2,m3;
	PVBNS bns= NULL;
	double dtemp;
	//double tt[2];//template uesed

	vgetparaline(l1->element.line,&a1,&b1,&c1);
	xc2 = cc2->element.arc->pc.x;
	yc2 = cc2->element.arc->pc.y;
	r2 = cc2->element.arc->r;
	xc3 = c3->element.arc->pc.x;
	yc3 = c3->element.arc->pc.y;
	r3 = c3->element.arc->r;
	if(CCW==l1->clockwise)m1 = -1;
	else m1 = 1;
	if(CCW==cc2->clockwise)m2 = 1;
	else m2 = -1;
	if(CCW==c3->clockwise)m3 = 1;
	else m3 = -1;
	m  = 2*(r3*m3-r2*m2);
	a2 = 2*(xc3-xc2);
	b2 = 2*(yc3-yc2);
	c2 = xc2*xc2-xc3*xc3+yc2*yc2-yc3*yc3+r3*r3-r2*r2;

	deta = a1*b2-b1*a2;
	if(fabs(deta)<=ZERO)// two parallel  line left for later write
	{//无解
		*i = 0;
		ASSERT(0);
		return;
	}
	if(fabs(a2)<=ZERO && fabs(b2)<=ZERO)//the circle with the same center;
	{
		double ta;
		if(m2 == m3)
		{
			*i = 0;
			return;
		}
		//
		ta = (r2-r3)/(m3-m2);
		//t is minor than zero is unbearable so kick it out without mercy
		if(ta<=-ZERO){
			*i = 0;
			return;
		}
		else 
		{
			vgainbisectorlc(l1,c3,&bns);
			dtemp = (bns->x5 + bns->x6*ta)*(bns->x5 + bns->x6*ta) -
				(bns->x7 + bns->x8*ta)*(bns->x7 + bns->x8*ta);
			delete bns; bns = NULL;
			if(dtemp<-ZERO){
				*i=0;
			}
			else{
				*i = 1;
				t[0] = ta;
			}
			return;
		}
	}
	x0 = (b1*c2-b2*c1)/deta;
	x1 = (m*b1-b2*m1)/deta;
	y0 =-(a1*c2-a2*c1)/deta;
	y1 =-(a1*m-a2*m1)/deta;
	 
	a = x1*x1+y1*y1-1;
	b = 2*(x1*(x0-xc3)+y1*(y0-yc3)-r3*m3);
	c = (x0-xc3)*(x0-xc3)+(y0-yc3)*(y0-yc3)-r3*r3;

	vgetvalue(a,b,c,t,i);
	return ;
}
//i=0,1,or 2;//i!=2?
void vgainintersectccc(PVCNS c1,PVCNS c2,PVCNS c3,int *i,double t[2])
{
	ASSERT(c1);ASSERT(c2);ASSERT(c3);
	ASSERT(CAXARC==c1->type);
	ASSERT(CAXARC==c2->type);ASSERT(CAXARC==c3->type);	
	//
	double xc1,yc1,r1,xc2,yc2,r2,xc3,yc3,r3;
	double a1,b1,cc1,mt1,mt2,a2,b2,cc2;
	double a,b,c,deta,x0,x1,y0,y1;
	int m1,m2,m3;
	PVBNS bns=NULL;
	double dtemp;
	//double tt[2];//template uesed

	xc1 = c1->element.arc->pc.x;
	yc1 = c1->element.arc->pc.y;
	r1 = c1->element.arc->r;
	xc2 = c2->element.arc->pc.x;
	yc2 = c2->element.arc->pc.y;
	r2 = c2->element.arc->r;
	xc3 = c3->element.arc->pc.x;
	yc3 = c3->element.arc->pc.y;
	r3 = c3->element.arc->r;
	if(CCW==c1->clockwise)m1 = 1;
	else m1 = -1;
	if(CCW==c2->clockwise)m2 = 1;
	else m2 = -1;
	if(CCW==c3->clockwise)m3 = 1;
	else m3 = -1;
	mt1 = 2*(r1*m1-r2*m2);
	a1  = 2*(xc1-xc2);
	b1  = 2*(yc1-yc2);
	cc1 = xc2*xc2-xc1*xc1+yc2*yc2-yc1*yc1+r1*r1-r2*r2;

	mt2 = 2*(r1*m1-r3*m3);
	a2  = 2*(xc1-xc3);
	b2  = 2*(yc1-yc3);
	cc2 = xc3*xc3-xc1*xc1+yc3*yc3-yc1*yc1+r1*r1-r3*r3;
	if(fabs(a1)<=ZERO&&fabs(a2)<=ZERO&&fabs(b1)<=ZERO&&fabs(b2)<=ZERO)//三点同心
	{
		*i = 0;ASSERT(0);
		return;
	}
	else if(fabs(a1)<=ZERO&&fabs(b1)<=ZERO)//两点同心
	{
		double ta;
		if(m1 == m2)
		{
			*i = 0;
			return;
		}
		ta = (r1-r2)/(m2-m1);
		if(ta<=-ZERO){
			*i = 0;
		}
		else 
		{
			vgainbisectorlc(c1,c3,&bns);
			dtemp = (bns->x5 + bns->x6*ta)*(bns->x5 + bns->x6*ta) -
				(bns->x7 + bns->x8*ta)*(bns->x7 + bns->x8*ta);
			delete bns; bns = NULL;
			if(dtemp<-ZERO){
				*i=0;
			}
			else{
				*i = 1;
				t[0] = ta;
			}
		}
		return;
	}
	else if(fabs(b2)<=ZERO&&fabs(a2<=ZERO))//两点同心
	{
		double ta;
		if(m1 == m3)
		{
			*i = 0;
			return;
		}
		//
		ta = (r1-r3)/(m3-m1);
		//t is minor than zero is unbearable so kick it out without mercy
		if(ta<=-ZERO){
			*i = 0;
		}
		else 
		{
			vgainbisectorlc(c2,c3,&bns);
			dtemp = (bns->x5 + bns->x6*ta)*(bns->x5 + bns->x6*ta) -
				(bns->x7 + bns->x8*ta)*(bns->x7 + bns->x8*ta);
			delete bns; bns = NULL;
			if(dtemp<-ZERO){
				*i=0;
			}
			else{
				*i = 1;
				t[0] = ta;
			}
		}
		return;
	}
	// above is the processing of the ccp 
	//after the function we can make sure that the intersect point must be the validated one
	//ok?
	deta = a1*b2 - b1*a2;
	if(fabs(deta)<ZERO)// two parallel  line left for later write
	{  //三点一线,好象是三个圆并排在一条直线，不，这也有交点。
		ASSERT(0);
		*i=0;
		return;
	}
	x0 = (b1*cc2-b2*cc1)/deta;
	x1 = (mt2*b1-b2*mt1)/deta;
	y0 =-(a1*cc2-a2*cc1)/deta;
	y1 =-(a1*mt2-a2*mt1)/deta;
	 
	a = x1*x1+y1*y1-1;
	b = 2*(x1*(x0-xc3)+y1*(y0-yc3)-r3*m3);
	c = (x0-xc3)*(x0-xc3)+(y0-yc3)*(y0-yc3)-r3*r3;

	vgetvalue(a,b,c,t,i);
	return ;
}
void  vgainintersect(PVCNS c1,PVCNS c2,PVCNS c3,int *i,double t[2])
{
	int t1,t2,t3 ;
	t1=c1->type ;
	t2=c2->type ;
	t3=c3->type ;
	if(CAXLINE==t1){
		if(CAXLINE==t2){
			if(CAXLINE==t3)
				vgainintersectlll(c1,c2,c3,i,t);
			else if(CAXARC==t3)
				vgainintersectllc(c1,c2,c3,i,t);
		}
		else if(CAXARC==t2)
		{
			if(CAXLINE==t3)
				vgainintersectllc(c1,c3,c2,i,t);
			else if(CAXARC==t3)
				vgainintersectlcc(c1,c2,c3,i,t);
		}
	}
	else if(CAXARC==t1)
	{
		if(CAXLINE==t2){
			if(CAXLINE==t3)
				vgainintersectllc(c2,c3,c1,i,t);
			else if(CAXARC==t3)
				vgainintersectlcc(c2,c3,c1,i,t);
		}
		else if(CAXARC==t2)
		{
			if(CAXLINE==t3)
				vgainintersectlcc(c3,c1,c2,i,t);
			else if(CAXARC==t3)
				vgainintersectccc(c1,c2,c3,i,t);
		}
	}
}
double vdis(double x1, double y1, double x2,double y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}
//得到T对应的M值
void vgainintersectmvaluebns(PVBNS bns1,PVBNS bns2,double *t,int mblis,int * mvalue)
{//mblis is corrispondent to the bns2,and if bns1 =ccp or llp then t is useless
	ASSERT(bns1);ASSERT(bns2);
	double detaa,detab,dtemp,dt,tt;
	double tfn,tsn;
	double tfny,tsny,tfpy,tspy,tfp;
	XYZ p;
	tt = *t;
	dt = tt * mblis ;
	//
	if(LL == bns1->bitype)
	{
		*mvalue = 1;
		return;
	}
	///////////gain tsn and tsny
	if(bns2->bitype == LLP){
		tsn  = bns2->x4 + bns2->x2 * dt;
		tsny = bns2->y4 + (-1)*bns2->x1 * dt;
	}
	else if(bns2->bitype == CCP){//t represent the value of alph
		tsn  = bns2->x1 + bns2->x2*cos(dt);
		tsny = bns2->y1 + bns2->x2*sin(dt);
	}
	else if(bns2->bitype == LL){
		tsn  = bns2->x1 - bns2->x3 * dt;//if is ll or lc or cc
		tsny = bns2->y1 - bns2->y3 * dt;//mblis ==1 
	}
	else {
		detab = pow(bns2->x5+bns2->x6*tt,2) - pow(bns2->x7+bns2->x8*tt,2);
		if(fabs(detab)<= ZERO){
			detab = 0;
		}
		else if(detab>ZERO){
			detab = sqrt(detab);
		}
		else ASSERT(0);
		tsn  = bns2->x1 -bns2->x2 - bns2->x3 * tt + mblis*bns2->x4*detab; 
		tsny = bns2->y1 -bns2->y2 - bns2->y3 * tt - mblis*bns2->y4*detab;
	}
	///////////////////
	if(bns1->bitype == LLP){
		if(fabs(bns1->x2)<=ZERO)
		{
			dtemp = (bns1->y4 - tsny )/bns2->x1;
		}
		else dtemp = (tsn - bns1->x4 )/bns2->x2;
		if(dtemp<0){
			*t = -dtemp;//for keep the same formation
			*mvalue = -1;
		}
		else if(dtemp>=0)
		{
			*t = dtemp;
			*mvalue = 1;
		}
		return;
	}
	else if(bns1->bitype == CCP){
		p.x = tsn;
		p.y = tsny;
		dtemp = vgainangleofvector(&p);
		*t = dtemp;//the fist alph value 
		*mvalue = 1;
		return;
	}
	///////////////////
	//////////////////////////above will 
	if(LC == bns1->bitype || CC == bns1->bitype)
	{
		if(bns2->bitype == LLP||bns2->bitype == CCP)
			tt = bns2->tmin;
		else tt = *t;
		detaa = pow(bns1->x5+bns1->x6*tt,2) - pow(bns1->x7+bns1->x8*tt,2);
		if(detaa<-ZERO)ASSERT(0);
		if(fabs(detaa) <=ZERO)
		{
			*mvalue = 1;
			return ;
		}
		detaa = sqrt(detaa);
		tfn  = bns1->x1 - bns1->x2 - bns1->x3*tt - bns1->x4*detaa;
		tfny = bns1->y1 - bns1->y2 - bns1->y3*tt + bns1->y4*detaa;
		tfn  = bns1->x1 - bns1->x2 - bns1->x3*tt + bns1->x4*detaa;
		tfny = bns1->y1 - bns1->y2 - bns1->y3*tt - bns1->y4*detaa;
		if(vdis(tsn,tsny,tfn,tfny)<=ZERO){
			*mvalue = -1;
			return;
		}
		else if(vdis(tsn,tsny,tfp,tfpy)<=ZERO){
			*mvalue = 1;
			return;
		}
	}
}
void  vgainintersectmvaluecns(PVBNS bns,PVCNS c1,PVCNS c2,PVCNS c3,double t,int *mvalue)
{
	if(LL==bns->bitype)
	{
		*mvalue = 1;
		return;
	}
	if(LLP==bns->bitype){}
	if(LC == bns->bitype||CC == bns->bitype)
	{
	}
	else if(CCP == bns->bitype){}
}
////..............................................................................
int  vgainbisectorlimitedll(PVCNS l1, PVCNS l2, PVBNS *bi)
{
	PVBNS bt;
	double t,t1,t2,t3,t4,tt0,tt1;
	double a1,a2,b1,b2,c1,c2;
	vgainbisectorll(l1,l2,&bt);
	if(bt==NULL)
	{
		*bi=bt;
		return 0;
	}
	//以下对t1,t2赋值
	vgetparaline(l1->element.line,&a1,&b1,&c1);
	vgetparaline(l2->element.line,&a2,&b2,&c2);
	t1 = (bt->x1 - l1->element.line->p0.x)/(a1 - bt->x3);
	t2 = (bt->x1 - l1->element.line->p1.x)/(a1 - bt->x3);
	t3 = (bt->x1 - l2->element.line->p0.x)/(a2 - bt->x3);
	t4 = (bt->x1 - l2->element.line->p1.x)/(a2 - bt->x3);
	if(t1>t2)
	{
		t=t1; 
		t1=t2;
		t2=t;
	}
	if(t3>t4)
	{
		t=t3;
		t3=t4;
		t4=t;
	}
	//
	if(fabs(t4-t3)<ZERO&&t4<=t2&&t4>=t1)
	{
		tt0=tt1=t4;
	}
	else if(fabs(t1-t2)<ZERO&&t1<=t4&&t1>=t3){ tt0=tt1=t1; }
	else if(t4<t1){ delete bt;*bi=NULL;return 0; }
	else if(t2<t3){ delete bt;*bi=NULL;return 0; }
	else if(t4>=t1 && t4<=t2)
	{
		if(t3<=t1)
		{
			tt0=t1;tt1=t4;
		}
		else
		{
			tt0=t3;tt1=t4;
		}
	}
	else if(t4>t2)
	{
		tt1=t2;
		if(t3<=t1)
		{
			tt0=t1;
		}
		else if(t3<=t2)
		{
			tt0=t3;
		}
	}
	bt->t1=tt0;
	bt->t2=tt1;
	bt->m1=1;
	bt->m2=1;
	*bi = bt;
	return 1;
}
//////........................................
int  vgainbisectorlimitedlc(PVCNS l1, PVCNS c2, PVBNS *bi)
{
	PVCNS cns;
	XYZ p;
	double t1,t2,t3,t4,t[2],tt1,tt2,xxa,xxb,h;
	int    m1,m2,m3,m4,i,mm1,mm2,clockwise;
	////...........................
	PVBNS bia,bib;
	vgainbisectorlc(l1,c2,&bia);
	//////
	p = l1->element.line->p0;
	vgainpointseg(&cns,&p);
	vgainintersectlcc(l1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1 - bia->x2 - bia->x3*t[0] + h;
	vgainbisectorlc(l1,cns,&bib);
	xxb = bib->x1 - bib->x2 - bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m1=1;
	else
		m1=-1;
	t1=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
	p = l1->element.line->p1;
	vgainpointseg(&cns,&p);
	vgainintersectlcc(l1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1-bia->x2-bia->x3*t[0]+h;
	vgainbisectorlc(l1,cns,&bib);
	xxb = bib->x1-bib->x2-bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m2=1;
	else
		m2=-1;
	t2=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
	if(m1==m2&&fabs(t1-t2)<ZERO)
	{
		delete bia;
		return 0;
	}
	///////////
	p = c2->element.arc->px*(c2->element.arc->r/10)+c2->element.arc->pc;
	vgainpointseg(&cns,&p);
	vgainintersectlcc(l1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1-bia->x2-bia->x3*t[0]+h;
	vgainbisectorcc(c2,cns,&bib);
	xxb = bib->x1-bib->x2-bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m3=1;
	else
		m3=-1;
	t3=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
//	vfindarcendpointo(c2->element.arc,&p,&clockwise);
	vgainpointseg(&cns,&p);
	vgainintersectlcc(l1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1-bia->x2-bia->x3*t[0]+h;
	vgainbisectorcc(c2,cns,&bib);
	xxb = bib->x1-bib->x2-bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO)
		m4=1;
	else
		m4=-1;
	t4=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
	if(m3==m4&&fabs(t4-t3)<ZERO)
	{
		delete bia;
		return 0;
	}
	//
	if(!vcomparerights(m1,t1,m2,t2,m3,t3,m4,t4,&mm1,&tt1,&mm2,&tt2))
	{
		delete bia;
		bia=NULL;
		return 0;
	}
	bia->m1=mm1;bia->t1=tt1;
	bia->m2=mm2;bia->t2=tt2;
	bia->infinite=LIMITED;
	return 1;
}
////////.............................................
int  vgainbisectorlimitedcc(PVCNS c1, PVCNS c2, PVBNS *bi)
{
	PVCNS cns;
	XYZ p;
	double t1,t2,t3,t4,t[2],tt1,tt2,xxa,xxb,h;
	int    m1,m2,m3,m4,i,mm1,mm2;
	////...........................
	PVBNS bia,bib;
	vgainbisectorcc(c1,c2,&bia);
	//
	p = c1->element.arc->px*(c1->element.arc->r/10)+c1->element.arc->pc;
	vgainpointseg(&cns,&p);
	vgainintersectccc(c1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1 - bia->x2 - bia->x3*t[0] + h;
	vgainbisectorlc(c1,cns,&bib);
	xxb = bib->x1 - bib->x2 - bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m1=1;
	else
		m1=-1;
	t1=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
//	vfindarcendpointo(c1->element.arc,&p,&clockwise);
	vgainpointseg(&cns,&p);
	vgainintersectccc(c1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1 - bia->x2 - bia->x3*t[0] + h;
	//
	vgainbisectorlc(c1,cns,&bib);
	xxb = bib->x1 - bib->x2 - bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m2=1;
	else
		m2=-1;
	t2=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	if(m2==m1&&fabs(t1-t2)<ZERO)
	{
		delete bia;
		return 0;
	}
	//
	p = c2->element.arc->px*(c2->element.arc->r/10)+c2->element.arc->pc;
	vgainpointseg(&cns,&p);
	vgainintersectccc(c1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1 - bia->x2 - bia->x3*t[0] + h;
	vgainbisectorlc(c2,cns,&bib);
	xxb = bib->x1 - bib->x2 - bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m3=1;
	else
		m3=-1;
	t3=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
//	vfindarcendpointo(c2->element.arc,&p,&clockwise);
	vgainpointseg(&cns,&p);
	vgainintersectccc(c1,c2,cns,&i,t);
	//
	if(i==2)AfxMessageBox("lcc find two solution");
	if(i==0)
	{
		delete bia;
		delete cns;
		return 0;
	}
	h=bia->x4*sqrt(pow(bia->x5+bia->x6*t[0],2)-pow(bia->x7+bia->x8*t[0],2));
	xxa = bia->x1 - bia->x2 - bia->x3*t[0] + h;
	//
	vgainbisectorlc(c2,cns,&bib);
	xxb = bib->x1 - bib->x2 - bib->x3*t[0];//h==0 means deta=0;
	if(fabs(xxa-xxb)<=ZERO*10)
		m4=1;
	else
		m4=-1;
	t4=t[0];
	delete cns;
	delete bib;
	cns=NULL;bib=NULL;
	//
	if(m3==m4&&fabs(t4-t3)<ZERO)
	{
		delete bia;
		return 0;
	}
	//
	if(!vcomparerights(m1,t1,m2,t2,m3,t3,m4,t4,&mm1,&tt1,&mm2,&tt2))
	{
		delete bia;
		bia=NULL;
		return 0;
	}
	bia->m1=mm1;bia->t1=tt1;
	bia->m2=mm2;bia->t2=tt2;
	bia->infinite=LIMITED;
	return 1;
}
int  vgainbisectorlimited(PVCNS c1, PVCNS c2, PVBNS *bi)
{
	PVBNS bt;
	if(CAXLINE==c1->type)
	{
		if(CAXLINE==c2->type)
			vgainbisectorlimitedll(c1,c2,&bt);
		else if(CAXARC==c2->type)
			vgainbisectorlimitedlc(c1,c2,&bt);
	}
	else if(CAXARC==c1->type)
	{
		if(CAXLINE==c2->type)
			vgainbisectorlimitedlc(c2,c1,&bt);
		else if(CAXARC==c2->type)
			vgainbisectorlimitedcc(c1,c2,&bt);
	}
	*bi=bt;
	if(bt==NULL)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
////////........................................................
//////////////////////.............................................................
//make sure the cnbl  must be the double link
//进行环的转换
double vmultipointvector(PXYZ v1,PXYZ v2)
{
	return v1->x*v2->x+v1->y*v2->y;
}
double vmultiplayvector(PXYZ v1,PXYZ v2)
{
	return v1->x*v2->y-v1->y*v2->x;
}
double vgainangleofvector(PXYZ v)
{
	if(fabs(v->x)<ZERO)
	{
		if(v->y>ZERO)
			return PI/2;
		else if(v->y<-ZERO)
			return 1.5*PI;
		else 
			ASSERT(0);
	}
	else if(fabs(v->x)>ZERO)
	{
		if(v->x>ZERO)
		{
			if(v->y>ZERO)
			    return atan(v->y/v->x);
			else if(v->y<-ZERO)
				return 2*PI+atan(v->y/v->x); 
			else 
				return 0;
		}
		else
		{
			   return PI+atan(v->y/v->x);
		}
	}
}
double vangleadd(double a1,double a2)
{
	double t;
	t=a1+a2;
	if(t>=PI*2)
		t=t-PI*2;
	if(t<0.)
		t=t+PI*2;
	return t;
}
void vfindarcendpoint(PARC arc,PXYZ *pt)//?
{
	ASSERT(arc);
	XYZ px,py,pc;
	PXYZ p=new XYZ;
	double a,r,alph,blph;
	int i;
	a=arc->a;
	r=arc->r;
	pc=arc->pc;
	px=arc->px-pc;
	py=arc->py-pc;
	alph=vgainangleofvector(&px);
	blph=vgainangleofvector(&py);
	if(fabs(vangleadd(alph,PI/2)-blph)<EP)
		i=1;
	else
		i=-1;
	alph=vangleadd(alph,i*a);
	*pt=p;
	p->x=pc.x+r*cos(alph);
	p->y=pc.y+r*sin(alph);
	p->z=0;
	return;
}
void vfindarcendpointo(PARC arc,PXYZ *pt,int *clockwise)
{
	ASSERT(arc);
	XYZ px,py;
	PXYZ p = new XYZ;
	double alph,blph;
	px = arc->px - arc->pc;
	py = arc->py - arc->pc; 
	alph = vgainangleofvector(&px);
	blph = vgainangleofvector(&py);
	double t = vangleadd(alph,PI/2);
	if(fabs(t-blph)<=ZERO*10||(blph>t&&2*PI-blph+t<=ZERO*10)||(blph<t&&2*PI -t+blph<=ZERO*10))
	{
		*clockwise = 1;//CCW
	}
	else
	{
			t = vangleadd(alph,-PI/2);
			if(fabs(t-blph)<=ZERO*10||(blph<t&&2*PI-t+blph<=ZERO*10)||
				(blph>t&&2*PI-blph+t<=ZERO*10))
			{
				*clockwise = -1;//CW
			}
			else ASSERT(0);
	}
	alph = vangleadd(alph,*clockwise*arc->a);
	*pt = p;
	p->x = cos(alph);
	p->y = sin(alph);
	p->z = 0;
	return;
}
int  vgetclockwisebli(PVBLI bli)
{
	ASSERT(bli);
	ASSERT(bli->bni);ASSERT(bli->bni->bns);
	PVBNS bns;
	bns = bli->bni->bns;
	double tm1,tm2,t;
	int m;
	tm1 = bns->t1 * bns->m1;
	tm2 = bns->t2 * bns->m2;
	if(tm2 - tm1<-ZERO)
	{
		m = bns->m1;
		t = bns->t1;
		bns->m1 = bns->m2;
		bns->t1 = bns->t2;
		bns->t2 = t;
		bns->m2 = m;
		return CW;
	}
	else if(fabs(tm1-tm2)<ZERO)
	{
		//can be writed in the funture
	}
	return CCW;
}
//find t
/*
int vjudgepointinareal(PXYZ p,XYZ p0,XYZ p1)
{
	XYZ v1,v2,v3;
	double t,t1,t2;
	v1=*p-p0;
	v2=p1-p0;
	v3=*p-p1;
	t  = vmultipointvector(&v1,&v2);
	t1 = vmultiplayvector(&v1,&v2);
	t2 = vmultpointvector(&v3,&v1);
	if(t1<-ZERO||t<-ZERO||t2>ZERO)
		return 0;
	if(t>ZERO&&t2<-ZERO&&t>ZERO)
		return 1;
	if(t>-ZERO&&t<ZERO)
	{
		return 2;//end in first point
	}
	if(t2>-ZERO&&t2<ZERO)
	{
		return 3;//end in end point
	}
}
int vjudgepointinareaa(PXYZ p,XYZ pc,XYZ px, XYZ py,double r)
{
	XYZ  v;
	double t,tf,te;
	v = arc->px-arc->pc;
	tf = vgainangleofvector(v);
	v = *p-arc->pc;
	t = vgainangleofvector(v);
	te = tf+arc->a;
	if(te<PI*2)
	{
		if(fabs(tf-t)<ZERO)
			return 2;
		if(fabs(te-t)<ZERO)
			return 3;
		if(t<te&&t>tf)
			return 1;
	}
	else
	{
		te=te-PI*2;
		if(fabs(tf-t)<ZERO)
			return 2;
		if(fabs(te-t)<ZERO)
			return 3;
		if((t<te)||t>tf)
			return 1;
	}
}
int vjudgepointinarea(PXYZ p,PVCNS cns)
{
	ASSERT(p);ASSERT(cns);
	XYZ p1,p2;
	int clocwise,type,juge;
	type=cns->type;
	clocwise=cns->clockwise;
	if(CAXLINE==type)
	{
		if(clowise==CCW)
		{
			judge = vjudgepointinareal(p,cns->element.line->px,cns->element.line->py);
			return judge;
		}
		else if(clocwise==CW)
		{
			judge = vjudgepointinareal(p,cns->element.line->py,cns->element.line->px);
			return judge;
		}
	}
	if(CAXARC==type)
	{
		if(clowise==CCW)
		{
			judge=vjudgepointinareaa(p,cns->element.arc);
		}
		else if(clowise==CW)
		{
		}
	}
}
*/
//the arc's own direction,from pfirst to pend.
double vfindnearestarcpoint(PARC arc,int *i,int *iclockwise)
{//only for perpendi
	ASSERT(arc);
	XYZ px,py,pc;
	PXYZ p;
	double a,b,c;
	int clockwise;
	pc = arc->pc;
	px = arc->px - pc;
	vfindarcendpointo(arc,&p,&clockwise);
	*iclockwise = clockwise;
	py.x = p->x;
	py.y = p->y;
	delete p;p= NULL;
	//
	a = vgainangleofvector(&px);
	b = vgainangleofvector(&py);
	if(clockwise == CCW)
	{
		c = b - a;
		if(c<=-ZERO)
		{
			b = 2*PI + c; 
		}
		if(PI-a>=ZERO&&b-PI>= -ZERO)
		{
			*i = 1;
			return PI;
		}
		else
		{
			*i=0;
			return a;
		}
	}
	else if(clockwise == CW)
	{
		c = a - b;
		if(c<=-ZERO)
		{
			c = 2*PI + c; 
		}
		if(PI-b>=ZERO&&a-PI>= -ZERO)
		{
			*i = 1;
			return PI;
		}
		else
		{
			*i = 0;
			return a;
		}
	}
}
int vgainarcdirection(PARC arc)
{
	ASSERT(arc);
	XYZ px,py,pc;
	PXYZ p=new XYZ;
	double a,r,alph,blph;
	a=arc->a;
	r=arc->r;
	pc=arc->pc;
	px=arc->px-pc;
	py=arc->py-pc;
	alph=vgainangleofvector(&px);
	blph=vgainangleofvector(&py);
	if(fabs(vangleadd(alph,PI/2)-blph)<EP)
		return CCW;
	else
		return CW;
}
int  vlineintarc(PLINE l1,PARC a1,PSTR3D *p)
{
	PSTR3D o_p;
	o_p=NULL;
	int flag=ciInterLineArcAllActualOnPlane(l1, a1 , &o_p);
	*p = o_p;
	return flag;
}
//nead rewrited but it is sufficiant  if only line and without arc
void vjugedirection(PVCNBL cnbl,int *flag)
{//没有处理重复段，单段，自交段//I think it will be at fault.find it way out sir
	ASSERT(cnbl);ASSERT(cnbl->cns);
	PVCNBL cnblt = NULL,cnbl1=NULL,cnbl2=NULL;
	double x,y,x0,y0,alph,tempt;
	PXYZ   p;
	int cnblflag,clockwiseflag,cnblflag0 = 0,clockwise;
	if(cnbl->ccw == cnbl && CAXARC == cnbl->cns->type)//圆,弧
	{
		*flag = CCW;
		return;
	}
	else if(cnbl->ccw == cnbl && CAXARC!=cnbl->cns->type)
	{
		AfxMessageBox("find spline,please");
		*flag = ERRO;//erro = 0
		return;
	}
	cnblt=cnbl1=cnbl2=cnbl;
	if(CAXLINE == cnblt->cns->type)
	{
		x0 = cnblt->cns->element.line->p0.x;
		y0 = cnblt->cns->element.line->p0.y;
	}
	else if(CAXARC == cnblt->cns->type)
	{
		alph = vfindnearestarcpoint(cnblt->cns->element.arc,&cnblflag,&clockwise);
		if(cnblflag == 1)
		{
			x0 = cnblt->cns->element.arc->pc.x - cnblt->cns->element.arc->r;
			y0 = cnblt->cns->element.arc->pc.y;
			clockwiseflag = clockwise;
		}
		else if(cnblflag == 0)
		{
			x0 = cos(alph)*cnblt->cns->element.arc->r + cnblt->cns->element.arc->pc.x;
			y0 = sin(alph)*cnblt->cns->element.arc->r + cnblt->cns->element.arc->pc.y;
		}
	}
	else ASSERT(0);
	do
	{
		cnblt = cnblt->ccw;
		if(CAXLINE == cnblt->cns->type)
		{
			x = cnblt->cns->element.line->p0.x;
			y = cnblt->cns->element.line->p0.y;
			if(x-x0<-ZERO||(fabs(x-x0)<=ZERO&&y-y0>=ZERO))
			{
				x0 = x;
				y0 = y;
				cnbl1 = cnblt;
				cnblflag0 = 0;
			}
		}
		else if(CAXARC==cnblt->cns->type)
		{
			alph = vfindnearestarcpoint(cnblt->cns->element.arc,&cnblflag,&clockwise);
			if(cnblflag == 1)
			{
				x = cnblt->cns->element.arc->pc.x - cnblt->cns->element.arc->r;
				y = cnblt->cns->element.arc->pc.y;
				if(x-x0<-ZERO||(fabs(x-x0)<=ZERO&&y-y0>=ZERO))
				{
					x0 = x;
					y0 = y;
					cnbl1 = cnblt;
					clockwiseflag = clockwise;
					cnblflag0 = 1;//this mark that the arc have contain the leftest point.
				}
			}
			else if(cnblflag == 0)
			{//this all be the first point of the segment
				x = cos(alph)*cnblt->cns->element.arc->r + cnblt->cns->element.arc->pc.x;
				y = sin(alph)*cnblt->cns->element.arc->r + cnblt->cns->element.arc->pc.y;
				if(x-x0<-ZERO||(fabs(x-x0)<=ZERO&&y-y0>=ZERO))
				{
					x0 = x;
					y0 = y;
					cnbl1 = cnblt;
					cnblflag0 = 0;
				}
			}
		}
		else ASSERT(0);
	}while(cnblt!=cnbl);
	XYZ p0,p1,p2;//以下代码应该可以简化
	if(cnblflag0 == 1)//if we find the furtherest point in arc then
	{
		*flag = clockwiseflag;
		return ;
	}//below is the case that cnblflag0 = 0. 
	if(CAXARC == cnbl1->cns->type)
	{
		p2 = cnbl1->cns->element.arc->px - cnbl1->cns->element.arc->pc;
		vfindarcendpointo(cnbl1->cns->element.arc,&p,&clockwise);
		delete p; p = NULL;
		if(clockwise == CCW)
		{
			tempt=p2.x;
			p2.x = -p2.y;
			p2.y = tempt;
		}
		else if (clockwise == CW)
		{
			tempt=p2.x;
			p2.x  =  p2.y;
			p2.y  = -tempt;
		}
	}
	else if(CAXLINE==cnbl1->cns->type)
	{
		p2 = cnbl1->cns->element.line->p1 - cnbl1->cns->element.line->p0;
	}
	//
	if(CAXARC == cnbl1->cw->cns->type)
	{
		vfindarcendpointo(cnbl1->cns->element.arc,&p,&clockwise);
		if(clockwise == CCW)
		{
			p1.x = -p->y*10;
			p1.y = p->x*10;
		}
		else if(clockwise == CW)
		{
			p1.x =  p->y*10;
			p1.y = -p->x*10;
		}
		delete p;p=NULL;
	}
	else if(CAXLINE == cnbl1->cw->cns->type)
	{
		p1 = cnbl1->cw->cns->element.line->p1 - cnbl1->cw->cns->element.line->p0;
	}
	if(vmultiplayvector(&p1,&p2)<-ZERO)
	{
		*flag = CW;
	}
	else
	{
		*flag = CCW;
	}
    return;
}
void vreversecns(PVCNS cns,int cnstype)//finished
{//we make sure that the cn->reflex*cn->clockwise = CCW/1(环),CW/-1(岛)
//in the meantime we must make sure of the clockwise can use for bisector calculation
	ASSERT(cns);
	PVCNS cn;
	PXYZ  p;
	int clockwise;
	cn = cns;
	XYZ tp;
	if(CAXARC == cn->type)
	{
		vfindarcendpointo(cn->element.arc,&p,&clockwise);
		cn->element.arc->px.x = 10*p->x + cn->element.arc->pc.x;
		cn->element.arc->px.y = 10*p->y + cn->element.arc->pc.y;
		if(clockwise == CW)
		{
			cn->element.arc->py.x = -10*p->y + cn->element.arc->pc.x;
			cn->element.arc->py.y =  10*p->x + cn->element.arc->pc.y;
			cn->clockwise = cnstype * (-1);
			cn->reflex = -1;
		}
		else
		{
			cn->element.arc->py.x =  10*p->y + cn->element.arc->pc.x;
			cn->element.arc->py.y = -10*p->x + cn->element.arc->pc.y;
			cn->clockwise = cnstype;
			cn->reflex = 1;
		}
		delete p;
	}
	else if(CAXLINE == cn->type)
	{
		tp = cn->element.line->p0;
		cn->element.line->p0 = cn->element.line->p1;
		cn->element.line->p1 = tp;
		cn->clockwise = cnstype;
	}
}
void varrangecnbl(PVCNBL cnbl , int flag,int cnstype)
{
	PVCNBL cn,tcnbl;
	cn = cnbl;
	PXYZ p;
	int clockwise;
	if(flag == CCW)
	{
		do{
			if(cn->cns->type == CAXARC)
			{
				vfindarcendpointo(cn->cns->element.arc,&p,&clockwise);//only for the clockwise
				cn->cns->clockwise = -cnstype * clockwise;
				cn->cns->reflex = -clockwise;//
			}
			else
				cn->cns->clockwise = cnstype;
		}while(cn!=0&&cn!=cnbl);
	}
	else{
		do{
			tcnbl   = cn->ccw;
			cn->ccw = cn->cw;
			cn->cw  = tcnbl;
			vreversecns(cn->cns,cnstype);
			cn = cn->ccw;
		}while(cn!=0&&cn!=cnbl);
	}
}
//if cnbla and cnblb have reflex rlationship
//then below insert a reflex between them;
void vinsertpointseg(PVCNBL cnbla,PVCNBL cnblb)
{
	ASSERT(cnbla);ASSERT(cnblb);
    XYZ px,py,pc;
	PXYZ p;
	PVCNBL cnbl;
	PVCNS  c;
	int clockwise;
	cnbl=vmalloccnbl();
	cnbl->cns=vmalloccns();
	if(CAXLINE == cnblb->cns->type)
	{
		//if(cnblb->clockwise == CCW)
			px = cnblb->cns->element.line->p0;
		//else if (cnblb->clockwise == CW)
		//	px = cnblb->cns->element.line->p1;
	}
	else if(CAXARC==cnblb->cns->type)
	{
		//if(cnblb->clockwise == CCW)//all directions are in ccw met
		px = cnblb->cns->element.arc->px*(cnblb->cns->element.arc->r/10);
		//	else if(cnblb->clockwise == CW)
		//	{
		//		vfindarcendpointo(arc,&p,&clockwise);
		//}
	}
	vgainpointseg(&c,&px);
	cnbl->cns  = c;
	cnbl->cns->clockwise = cnbl->cns->clockwise;
	cnbl->cw   =cnbla;
	cnbl->ccw  =cnblb;
	cnbla->ccw =cnbl;
	cnblb->cw  =cnbl;
	return;
}
//judge whether the cnsf and cnse are reflex relation
//return REFLEX OF UNREFLEX;
int vjudgeangle(PVCNS cnsf,PVCNS cnse)
{
	ASSERT(cnsf);ASSERT(cnse);
	double t;
	XYZ p0,p1,p2;
	PXYZ p = NULL;
	int clockwise;
	if(CAXLINE == cnsf->type)
	{
		p1 = cnsf->element.line->p1 - cnsf->element.line->p0;
		if(CAXLINE == cnse->type)
		{
			p2 = cnse->element.line->p1 - cnse->element.line->p0;
		}
		else if(CAXARC == cnse->type)
		{
			vfindarcendpointo(cnse->element.arc,&p,&clockwise);
			delete p;p=NULL;
			p2 = cnse->element.arc->px - cnse->element.arc->pc;
			t = p2.x;
			if(clockwise == CCW)
			{
				p2.x = -p2.y;
				p2.y = t;
			}
			else{
				p2.x = p2.y;
				p2.y = -t;
			}
		}
	}
	else if(CAXARC == cnsf->type)
	{
		vfindarcendpointo(cnsf->element.arc,&p,&clockwise);
		p1.x = p->x*10;
		p1.y = p->y*10;
		delete p;p=NULL;
		t = p1.x;
		if(clockwise == CCW)
		{
			p1.x = -p1.y;
			p1.y = t;
		}
		else if (clockwise == CW)
		{
			p1.x = p1.y;
			p1.y = -t;
		}
		if(CAXLINE == cnse->type)
		{
			p2 = cnse->element.line->p1 - cnse->element.line->p0;
		}
		else if(CAXARC == cnse->type)
		{
			vfindarcendpointo(cnsf->element.arc,&p,&clockwise);
			delete p; p=NULL;
			p2 = cnse->element.arc->px - cnse->element.arc->pc;
			t = p2.x;
			if(clockwise == CCW)
			{
				p2.x = -p2.y;
				p2.y = t;
			}
			else if(clockwise == CW)
			{
				p2.x = p2.y;
				p2.y = -t;
			}
		}
	}
	t = vmultiplayvector(&p1,&p2);
	//if I let the line's reflex =1 then two if can be compact to one
	if(CAXARC == cnsf->type)
	{
		if(t*cnsf->clockwise*cnsf->reflex>=0)//=0 tangential 
			return UNREFLEX;
		else 
			return REFLEX;
	}
	else if(CAXLINE == cnsf->type)
	{
		if(t*cnsf->clockwise>=0)//=0 tangential
		{
			return UNREFLEX;
		}
		else 
		{
			return REFLEX;
		}
	}
}
//request cnbl'loop have been arranged,or cnbl'loop'node'direction
void  vchangepcurvetocnblloop(PCURVE curve,PVCNBL *cnbl)
{
	ASSERT(curve);
	PVCNBL cnblt,cnblstf,cnblb;
	PCURVE cur;
	cur = curve;
	do{
		if(curve==cur)
		{
			cnblstf = cnblt = vmalloccnbl();
			*cnbl=cnblstf;
			cnblt->cns = vmalloccns();
			cnblt->cns->type=cur->type;
			cnblt->cns->element=cur->element;
			cnblb=cnblt;
		}
		else
		{
			cnblt=vmalloccnbl();
			cnblt->cns=vmalloccns();
			cnblt->cns->type=cur->type;
			cnblt->cns->element=cur->element;
			cnblt->cw=cnblb;
			cnblb->ccw=cnblt;
			cnblb=cnblt;
		}
	    cur=cur->next;
	}while(cur);
	cnblb->ccw=cnblstf;
	cnblstf->cw=cnblb;
	return;
}
void vgainarrangedcnbl(PCURVE curve,PVCNBL *cnbl,int itype)
{
	PVCNBL cnblt;
	int flag;
	vchangepcurvetocnblloop(curve,&cnblt);
	vjugedirection(cnblt,&flag);
	varrangecnbl(cnblt,flag,itype);
	*cnbl=cnblt;
	return;
}
void vpreprossesscnbl(PVCNBL cnbl,PVPL *vpl)
{
	PVPL pl=NULL;
	PVPLF plfa=NULL,plfb=NULL,plf=NULL,plfsta;
	PVCNBL cnbla,cnblb,cnblsta,cnblf;
	PVCNS  cns,cnsa,cnsb;
	int flag = 1,label = 1,sum = 0;
	int flagangle;
	//
	cnblb = cnbl;
	cnbla = cnbl;
	cnblsta=cnbla;
	//
	do{
		cnsa = cnbla->cns;
		cnblf = cnbla->cw;
		cns = cnbla->cw->cns;
		flagangle = vjudgeangle(cns,cnsa);
		if(UNREFLEX == flagangle)
		{
			flag = 0;
		}
		else if(REFLEX==flagangle)
		{
			//insert an seg
			vinsertpointseg(cnblf,cnbla);
			if(cnblf==cnbl)
			{
				label=0;
			}
			else if(cnblf!=cnbl)
			{
				cnbla=cnblf;
			}
		}
	}while(flag&&label);
	//
	if(!label)//whole loop
	{
		pl=vmallocpl();
		plfa=vmallocplf();
		if(cnbl->ccw==cnbl)
			plfa->type=SINGLEPLF;
		else
			plfa->type=MULIPLF;
		plfa->scnbl = cnbla;
		plfa->ecnbl = cnbl;
		pl->ptype=SINGLE;
		pl->eplf=pl->splf=plfa;
		*vpl = pl;
		return;
	}
	else//not whole loop
	{
		if(cnbla->cw==cnbl)
		{
			pl=vmallocpl();
			plfa=vmallocplf();
			if(cnbl->cw==cnbla)
				plfa->type=SINGLEPLF;
			else
				plfa->type=MULIPLF;
			plfa->scnbl = cnbla->ccw;
			plfa->ecnbl = cnbl;
			plf = vmallocplf();
			plf->type=SINGLEPLF;
			plf->scnbl=plf->ecnbl=cnbla;
			plf->ccw=plfa;
			plf->cw=plfa;
			plfa->ccw=plfa->cw=plf;
			pl->eplf=plf;
			pl->splf=plfa;
			pl->ptype=MULI;
			*vpl=pl;
			return;
		}
		else{
			plfa=vmallocplf();
			plfa->scnbl=cnbla;
		}
	}
	flag=0;
	label=1;
	cnblb=cnbl;
	cnblsta=cnbla;
	plfsta=plfa;
	do{
		cnsb=cnblb->cns;
		cnblf=cnblb->ccw;
		cns=cnblb->ccw->cns;
		flagangle=vjudgeangle(cnsb,cns);
		//
		if(UNREFLEX==flagangle)
		{
			if(flag==0)
			{
				if(cnblb->ccw==cnbla)
				{
					pl=vmallocpl();
					plfa=vmallocplf();
					plfa->type=MULIPLF;
					plfa->scnbl = cnbla->ccw;
					plfa->ecnbl = cnblb;
					//
					plf = vmallocplf();
					plf->type=SINGLEPLF;
					plf->scnbl=plf->ecnbl=cnbla;
					//
					plf->ccw=plfa;
					plf->cw=plfa;
					plfa->ccw=plf;
					plfa->cw=plf;
					//
					pl->eplf=plf;
					pl->splf=plfa;
					pl->ptype=MULI;
					*vpl=pl;
					return;
				}
				plfsta->ecnbl=cnblb;
				if(cnblb==plfsta->scnbl)
					plfsta->type=SINGLEPLF;
				else
					plfsta->type=MULIPLF;
				//
				cnbla = cnblb->ccw;
				cnblb = cnbla;
				flag = 1;
			}
			else{
				if(cnblb->ccw==cnblsta)//end
				{
					plfb=vmallocplf();
					plfb->scnbl=cnbla;
					plfb->ecnbl=cnblb;
					if(cnbla!=cnblb)
					{
						plfb->type=MULIPLF;
					}
					else
					{
						plfb->type=SINGLEPLF;
					}
					plfb->cw=plfa;
					plfa->ccw=plfb;
					//
					plfsta->cw=plfb;
					plfb->ccw=plfsta;
					//
					pl=vmallocpl();
					pl->splf=plfsta;
					pl->eplf=plfb;
					if(plfb==plfsta)
					{
						pl->ptype=SINGLE;
					}
					else
					{
						pl->ptype=MULI;
					}
					//
					label=0;
					*vpl=pl;
				}
				else
				{
					//
					plfb=vmallocplf();
					plfb->scnbl=cnbla;
					plfb->ecnbl=cnblb;
					if(plfb->ecnbl==plfb->scnbl)
						plfb->type=SINGLE;
					else
						plfb->type=MULIPLF;
					//
					plfa->ccw =plfb;
					plfb->cw =plfa;
					plfa=plfb;
					//
					cnbla=cnblb->ccw;
					cnblb=cnbla;
				}
			}
		}
		///
		else if(REFLEX==flagangle)
		{
			vinsertpointseg(cnblb,cnblf);
			cnblb=cnblf;
		}
	}while(label);
	
}
void vgainprofilebycurve(PCURVE curve,PVPL *vpl,int itype)
{
	PVCNBL cnbl;
	PVPL  vplt;
	vgainarrangedcnbl(curve,&cnbl,itype);
	vpreprossesscnbl(cnbl,&vplt);
	*vpl=vplt;
	return;
}
////////////.................................................................
void vgainpointseg(PVCNS *c,PXYZ p)
{
	ASSERT(p);
	PVCNS cn;
	cn = vmalloccns(); 
	*c = cn;
	cn->element.arc = new ARC;
	cn->type = CAXARC;
	cn->element.arc->r = 0;
	cn->element.arc->a = 2*PI;
	cn->element.arc->pc.x = p->x;
	cn->element.arc->pc.y = p->y;
	cn->element.arc->pc.z = p->z;
 	return;
}
void vaddpointsegcnbl(PVCNS *c,PXYZ p,PVCNBL *cnbl)
{	
	ASSERT(p);
	PVCNBL tcnbl=NULL;
	//
	PVCNS cn=new VCNS;
	*c=cn;
    cn->clockwise=CCW;
	cn->element.arc=new ARC;
	cn->reflex=0;
	cn->type=CAXARC;
	cn->element.arc->r=0;
	cn->element.arc->pc.x=p->x;
	cn->element.arc->pc.y=p->y;
	cn->element.arc->pc.z=p->z;
	//
	tcnbl=new VCNBL;
	tcnbl->bli=new VBLI;
	tcnbl->cns=cn;
	tcnbl->bli->bni=new VBNI;
 	return;
}
void vaddpointsegpre(PVCNBL cnbl)//cw 方向
{
	ASSERT(cnbl);
	ASSERT(cnbl->cns);
	ASSERT(1!=cnbl->flag);

	XYZ px,py;
	PXYZ p;
	int clockwise;
	PVCNS cns = NULL;
	PVCNBL tcnbl = NULL,cnblt;
	tcnbl = vmalloccnbl();
	//
	if(CAXLINE == cnbl->cns->type)
	{
		px = cnbl->cns->element.line->p0;
		py = cnbl->cns->element.line->p1;
	}
	else if(CAXARC == cnbl->cns->type)
	{
		ASSERT(cnbl->cns->element.arc->r!=0);//perhaps if there are problem in reflex assert
		px = cnbl->cns->element.arc->px * (cnbl->cns->element.arc->r/10);
		vfindarcendpointo(cnbl->cns->element.arc,&p,&clockwise);
		py.x = p->x*cnbl->cns->element.arc->r + cnbl->cns->element.arc->pc.x;
		py.y = p->y*cnbl->cns->element.arc->r + cnbl->cns->element.arc->pc.y;
		delete p;p=NULL;
	}
	else 
		ASSERT(0);//not belong to line or arc
	if(cnbl->cns->type == CAXLINE)cnbl->cns->reflex = 1;
	if(CCW == cnbl->cns->clockwise*cnbl->cns->reflex)
	{
		vgainpointseg(&cns,&px);
	}
	else if(CW == cnbl->cns->clockwise*cnbl->cns->reflex)
	{
		vgainpointseg(&cns,&py);
	}
	cnblt = cnbl->cw;
	tcnbl->cns = cns;
	tcnbl->ccw = cnbl;
	cnbl->cw = tcnbl;
	tcnbl->cw = cnblt;
	tcnbl->flag = 1;
	return;
}
void vaddpointseglast(PVCNBL cnbl)
{
	ASSERT(cnbl);ASSERT(cnbl->cns);ASSERT(1!=cnbl->flag);
	XYZ px,py;
	PXYZ p;
	int clockwise;
	PVCNS cns = NULL;
	PVCNBL tcnbl = NULL,cnblt;
	tcnbl = vmalloccnbl();
	//
	if(CAXLINE == cnbl->cns->type)
	{
		px=cnbl->cns->element.line->p0;
		py=cnbl->cns->element.line->p1;
	}
	else if(CAXARC==cnbl->cns->type)
	{
		ASSERT(cnbl->cns->element.arc->r!=0);
		px = cnbl->cns->element.arc->px * (cnbl->cns->element.arc->r/10);
		vfindarcendpointo(cnbl->cns->element.arc,&p,&clockwise);
		py.x = p->x*cnbl->cns->element.arc->r + cnbl->cns->element.arc->pc.x;
		py.y = p->y*cnbl->cns->element.arc->r + cnbl->cns->element.arc->pc.y;
		delete p;p=NULL;
	}
	else 
		ASSERT(0);
	if(cnbl->cns->reflex == CAXLINE)cnbl->cns->reflex = 1;
	if(CCW == cnbl->cns->clockwise*cnbl->cns->reflex)
	{
		vgainpointseg(&cns,&py);
	}
	else if(CW==cnbl->cns->clockwise*cnbl->cns->reflex)
	{
		vgainpointseg(&cns,&px);
	}
	cnblt = cnbl->ccw;
	tcnbl->cns = cns;
	tcnbl->cw = cnbl;
	cnbl->ccw = tcnbl;
	tcnbl->ccw = cnblt;
	tcnbl->flag = 1;
	return;
}
//NOTES:I am sure that there are no zerolenght line,duplicity line or arc;
//在两边都加上平分线
void vgainsinglebisector(PVCNBL cnbl,PVBLI *obli)
{
	ASSERT(cnbl);
	PVBLI blis=NULL,blie=NULL,blit = NULL;//blis mean start bli,blie mean end bli
	PVBNI bni=NULL,bnit=NULL;
	PVCNS cns=NULL,cnst=NULL,cnse = NULL;
	PVBNS bns=NULL;
	XYZ  px,py;
	PXYZ p;
	int clockwise;
	//
	cns = cnbl->cns;
	//
	if(CAXLINE == cns->type)
	{
		px = cns->element.line->p0;
		//
		vgainpointseg(&cnst,&px); ASSERT(cnst);
		vgainbisectorlc(cns,cnst,&bns); ASSERT(bns);
		bns->t1 = 0;
		blis = vmallocbli();
		blis->bni = vmallocbni();
		blis->bni->selfcns = cnbl->cns;
		blis->bni->bns = bns;
		//
		bns = NULL;cnst = NULL;
		px = cns->element.line->p1;
		vgainpointseg(&cnst,&px); ASSERT(cnst);
		vgainbisectorlc(cns,cnst,&bns); ASSERT(bns);
		bns->t1 = 0;
		blie = vmallocbli();
		blie->bni = vmallocbni();
		blie->bni->selfcns = cnbl->cns;
		blie->bni->bns = bns;
		//
		bns=NULL;cnst=NULL;
		if(CCW == cns->clockwise)
		{
			*obli = blie;
			blie->clockwise = CCW;
			blis->clockwise = CW;
			blie->next = blis;
			blis->last = blie;
		}
		else if(CW == cns->clockwise)
		{
			*obli = blis;
			blis->clockwise = CCW;
			blie->clockwise = CW;
			blis->next = blie;
			blie->last = blis;
		}
	}
	else if(CAXARC == cns->type)
	{
		if(fabs(cns->element.arc->r)>ZERO)
		{
			px = (cns->element.arc->px - cns->element.arc->pc)*(cns->element.arc->r/10)
				 + cns->element.arc->pc ;
			vfindarcendpointo(cns->element.arc,&p,&clockwise);
			py.x = p->x * cns->element.arc->r + cns->element.arc->pc.x;
			py.y = p->y * cns->element.arc->r + cns->element.arc->pc.y;
			delete p;p=NULL;
			//
			vgainpointseg(&cnst,&px);ASSERT(cnst);
			vgainbisectorcc(cnst,cns,&bns);ASSERT(bns);
			bns->t1 = 0;
			blis = vmallocbli();
			blis->bni = vmallocbni();
			blis->bni->selfcns = cnbl->cns;
			blis->bni->bns = bns;
			bns = NULL;cnst = NULL;
			//
			vgainpointseg(&cnse,&py);ASSERT(cnse);
			vgainbisectorcc(cnse,cns,&bns);ASSERT(bns);
			bns->t1 = 0;
			//
			blie=vmallocbli();
			blie->bni=vmallocbni();
			blie->bni->selfcns = cnbl->cns;//I think this number will be deleted
			blie->bni->bns = bns;
			bns = NULL;cnst = NULL;
			//
			if(cns->clockwise == CW){
				blie->bni->bns->t2 = cns->element.arc->r;
				blis->bni->bns->t2 = cns->element.arc->r;
			}
			if(CCW == cns->clockwise*cns->reflex)
			{
				*obli = blie;
				blie->clockwise = CCW;
				blis->clockwise = CW;
				blie->next = blis;
				blis->last = blie;
			}
			else if(CW == cns->clockwise*cns->reflex)
			{
				*obli = blis;
				blis->clockwise = CCW;
				blie->clockwise = CW;
				blis->next = blie;
				blie->last = blis;
			}
		}
	}
	return;
}
///////////////////////////////////////
///////////////////////////////////////
//得到单树枝，即只有一个树叶的树枝，对其树叶进行赋bisector
//没有任何角平分线信息，头尾也没有新加节点
//function
//加上头尾节点，加上角平分线连，加上角平分线对应关系
/////////////////////////////
void vgainsingleplfbli(PVPLF plf)
{
	ASSERT(plf);
	PVBLI  bli = NULL,tbli = NULL;
	PVCNBL tcnbl = NULL;
	PVCNBL cnbls = NULL,cnble = NULL;
	//是单节点树叶 it's impossible to have zero segment
	if(SINGLEPLF == plf->type)
	{ 
		vgainsinglebisector(plf->scnbl,&bli);//得到节点的bisector连
		plf->scnbl->bli=bli;
		vaddpointsegpre(plf->scnbl);//在前加上点节点note 没有bli的信息
		vaddpointseglast(plf->ecnbl);//在后加上点节
		//
		cnbls = plf->scnbl->cw;
		cnble = plf->ecnbl->ccw;
		//
		tbli = bli;
		while(tbli->next)
		{
			tbli=tbli->next;
		}
		//
		cnbls->bli=vmallocbli();
		cnbls->bli->bni = vmallocbni();
		cnble->bli = vmallocbli();
		cnble->bli->bni = vmallocbni();
		//
		cnbls->bli->bni->bns  = tbli->bni->bns;
		cnbls->bli->clockwise =-tbli->clockwise;//?
		cnble->bli->bni->bns  = bli->bni->bns;
		cnble->bli->clockwise = -bli->clockwise;//?
		//
		cnbls->bli->bni->othercns = cnbls->ccw->cns;
		cnble->bli->bni->othercns = cnble->cw->cns;
		plf->ecnbl->bli->bni->othercns = cnble->cns;
		plf->ecnbl->bli->next->bni->othercns = cnbls->cns;
		//
		plf->scnbl=cnbls;
		plf->ecnbl=cnble;
		//
		return;
	}//多节点 have the oppotunity to be an point segment
	plf->type = MULIPLF;
	//
	tcnbl = plf->scnbl;
	vaddpointsegpre(tcnbl);
	tcnbl->cw->bli = vmallocbli();
	tcnbl->cw->bli->bni = vmallocbni();
	//
	do
	{
		if(tcnbl->cns->type==CAXARC&&fabs(tcnbl->cns->element.arc->r)<=ZERO)//是点
		{
			tcnbl->bli = vmallocbli();
			tcnbl->bli->bni = vmallocbni(); 
			tcnbl->bli->next = vmallocbli();
			tcnbl->bli->next->bni = vmallocbni();
			tcnbl->bli->next->last = tcnbl->bli;
			//
			tcnbl->bli->next->bni->othercns = tcnbl->cw->cns;
			tcnbl->cw->bli->bni->othercns = tcnbl->cns;
			//
			tcnbl->bli->next->bni->bns = tcnbl->cw->bli->bni->bns;
			tcnbl->bli->next->clockwise = -tcnbl->cw->bli->clockwise;
			tcnbl = tcnbl->ccw;
		}
		else
		{
			vgainsinglebisector(tcnbl,&bli);
			ASSERT(bli);
			tcnbl->bli = bli;
			//
			tcnbl->cw->bli->bni->othercns = tcnbl->cns;
			tcnbl->bli->next->bni->othercns = tcnbl->cw->cns;
			//
			tcnbl->cw->bli->bni->bns = bli->next->bni->bns;
			tcnbl->cw->bli->clockwise = -bli->next->clockwise;
			tcnbl = tcnbl->ccw;
		}
		tcnbl->bli=bli;
		bli=NULL;
	}while(tcnbl != plf->ecnbl);
	//
	vgainsinglebisector(tcnbl,&bli);
	ASSERT(bli);
	tcnbl->bli = bli;
	//
	tcnbl->cw->bli->bni->bns = bli->next->bni->bns;
	tcnbl->cw->bli->clockwise = -bli->next->clockwise;
	//
	tcnbl->cw->bli->bni->othercns = tcnbl->cns;
	tcnbl->bli->next->bni->othercns = tcnbl->cw->cns;
	//给最后的一个节点进行赋值
	vaddpointseglast(tcnbl);
	tcnbl = tcnbl->ccw;
	tcnbl->bli = vmallocbli();
	tcnbl->bli->bni = vmallocbni();
	//
	tcnbl->bli->bni->bns = tcnbl->cw->bli->bni->bns;
	tcnbl->bli->clockwise = -tcnbl->cw->bli->clockwise;
	tcnbl->bli->bni->othercns = tcnbl->cw->cns;
	//
	tcnbl->cw->bli->bni->othercns = tcnbl->cns;
	//
	plf->scnbl=plf->scnbl->cw;
	plf->ecnbl=plf->ecnbl->ccw;
	//
	return;
}
//把一个树枝分成两个近似相等的树枝
//ok
void vdivideprofiletwo(PVPL vpl,PVPL *pl,PVPL *pr)
{
	ASSERT(vpl);ASSERT(vpl->ptype!=SINGLE);
	ASSERT(vpl->splf);
	PVPL kpl=NULL;
	int sum=1;
	PVPLF plf=NULL;
	plf = vpl->splf;
	while(plf != vpl->eplf)
	{
		sum++;
		plf=plf->ccw;
	}
	plf = vpl->splf;
	if(sum==2)
	{
		kpl=new VPL;ASSERT(kpl);
		kpl->eplf = kpl->splf = vpl->eplf;
		kpl->ptype=SINGLE;
		*pl=kpl;
		kpl=NULL;
		kpl=new VPL;ASSERT(kpl);
		kpl->eplf = kpl->splf = vpl->splf;
		kpl->ptype=SINGLE;
		*pr=kpl;
		kpl=NULL;
	}
	else if(sum==3)
	{
		kpl = new VPL;ASSERT(kpl);
		kpl->splf = vpl->eplf->cw;
		kpl->eplf = vpl->eplf;
		kpl->ptype = MULIPLF;
		*pl = kpl;
		kpl = NULL;
		kpl = new VPL;ASSERT(kpl);
		kpl->eplf = kpl->splf = vpl->splf;
		kpl->ptype = SINGLE;
		*pr = kpl;
		kpl = NULL;
	}
	else if(sum>3)
	{
		sum = sum/2;
		for(int i=1;i<sum;i++)
			plf = plf->ccw;
		kpl = new VPL;ASSERT(kpl);
		kpl->splf = plf->ccw;
		kpl->eplf = vpl->eplf;
		kpl->ptype = MULIPLF;
		*pl = kpl;
		kpl = NULL;
		kpl = new VPL; ASSERT(kpl);
		kpl->splf = vpl->splf;
		kpl->eplf = plf;
		kpl->ptype = MULIPLF;
		*pr=kpl;
		kpl=NULL;
	}
	delete vpl;   //?
	vpl=NULL;
	return;
}
///////////////////////////////////////////////////////////////////////////
///merge
///////////////////////////////////////////////////////////////////////////
//find whether the t is in the limitation of bns 
//rcnbl=vfindcnblr(blir->bni->othercns,rcnbl)
//找到blil的另一部分cns对应的cnbl节点
PVCNBL vfindcnblr(PVCNS cns,PVCNBL rcnbl)
{
	PVCNBL cnbl;
	cnbl = rcnbl;
	while(cnbl->cns != cns)
		cnbl = cnbl->cw;
	return cnbl;
}
//找到blil的另一部分cns对应的cnbl节点
PVCNBL vfindcnbll(PVCNS cns,PVCNBL lcnbl)
{
	PVCNBL cnbl;
	cnbl=lcnbl;
	while(cnbl->cns != cns)
		cnbl=cnbl->ccw;
	return cnbl;
}
////.............................................
//判断t,m 是否在bns中 
//return 0,mean t is not in bns
//return 1,mean only in the end point of bns
//return 2,mean in the middle of bns
int vjudgetheinbns(PVBNS bns,double t,int m)
{
	ASSERT(bns);//ASSERT(bns->t1<=bns->t2);
	double tm1,tm2,tm;
	double tt;
	//
	tm = t * m;
	tm1 = bns->t1 * bns->m1;
	tm2 = bns->t2 * bns->m2;
	if((t - bns->tmin)<-ZERO)// t不能小于tmin>=0
	{
		return 0;
	}
	else if(fabs(t-bns->tmin)<ZERO)
	{
		return 1;
	}
	//arrange ,are the bns arrange before?
	//if be arranged bellow can be deleted safely
	if(fabs(tm1-tm2)<=ZERO){ASSERT(0);return 0;}//点角平分线处理
	else if(tm1-tm2>ZERO)
	{
		tt=tm1;
		tm1=tm2;
		tm2=tt;
	}
	//
/*	if(INFINITY == bns->infinite) //下面是两个未arrange的算法
	{
		if(bns->t1 == UPER&&bns->t2 == UPER)
			return 2;
		else if(bns->t1 != UPER)
		{
			if(fabs(tm1-tm)<=ZERO)
				return 1;
			else if(tm-tm1>ZERO)
				return 2;
			else return 0;
		}
		else// bns->t2!=UPPER 
		{
			if(fabs(tm2-tm)<=ZERO)
				return 1;
			else if(tm2-tm>ZERO)
				return 2;
			else return 0;
		}
	}
	else//neither bns->t1 nor bns->t2 ==uper  
*/	{
		if(fabs(tm - tm1)<=ZERO||fabs(tm-tm2)<=ZERO)return 1;
		else if(tm-tm1>ZERO&&tm-tm2<-ZERO)
			return 2;
		else return 0;		
	}
}
//
PVBLI vfindimlimitedbli(PVBLI bli,int flag)
{
	PVBLI blit=NULL;
	int m;
	if(flag ==2)//meet with right
	{
	    blit = bli;
		while(blit->next)
		{
			blit = blit->next;
		}
	}
	else if(flag == 1)//meet with left
	{
		blit = bli;
		while(blit->last)
		{
			blit = blit->last;
		}
	}
	while(blit)
	{
		if((blit->clockwise==CCW && flag == 1)||(blit->clockwise==CW && flag==2))
		{
			if(blit->bni->bns->t2 == UPER)
				break;
		}
		else if((blit->clockwise==CW && flag==1)||(blit->clockwise==CCW && flag==2))
		{
			if(blit->bni->bns->t1 ==UPER)
				break;
		}
			if(flag == 2)
				blit = blit->last;
			else if(flag == 1)
				blit = blit->next;
	}
	if(blit)
		return blit;
	else
		ASSERT(0);
	return NULL;
}
//for unexpected dealing with
//make sure that pbl>=0,rcnblin!=NULL,and so on and the blistart is the first bli needed
void vregendercnblunexpectedl(PVCNBL *lcnblin,PVCNBL rcnbl,PVBLI blistart,PVBLI bliend,
					PVBLI pbufl[],int pbl,PVBLI *bliout)
{
	PVCNBL lcnbl = *lcnblin;
	PVBLI  blia  = NULL;
	int i;
	if(lcnbl->flag == 1)
	{
		if(pbl == 0)
		{
			pbufl[0]->clockwise = -pbufl[0]->clockwise;
			lcnbl->bli = pbufl[0];
			pbufl[0]->next = blistart;
			if(blistart)
			{
				blistart->last = pbufl[0];
			}
			else{
				ASSERT(0);
			}
			return ;
		}
		for( i = pbl; i>=0; i--)
		{
			pbufl[i]->clockwise *=-1;
			if(i==pbl)
			{
				lcnbl->bli = pbufl[i];
			}
			else if (i==0)
			{
				pbufl[i]->last = pbufl[i+1];
				pbufl[i+1]->next = pbufl[i];
				//
				pbufl[i]->next = blistart;
				if(blistart != NULL)
				{
					blistart->last = pbufl[i];
				}
			}
			else
			{
				pbufl[i]->last = pbufl[i+1];
				pbufl[i+1]->next = pbufl[i];
			}
		}
		return;
	}
	else
	{
		//above is the finding of the first/(last in sequential meaning) bli
		blia = vfindimlimitedbli(blistart,1);
		if(pbl == 0)
		{
			pbufl[0]->next = blistart;
			pbufl[0]->clockwise *=-1;//it is so important that it safe to never touch it
			if(blistart){
				blistart->last = pbufl[0];
			}
			else ASSERT(0);
			if(blia)
			{
				pbufl[0]->last = blia;
				blia->next = pbufl[0];
			}
			else
			{
				lcnbl->bli = pbufl[0];
			}
		}
		else{//为伊消得人憔悴
			for(i=0;i<=pbl;i++)
			{
				pbufl[i]->clockwise *=-1;
				if(i == 0)
				{
					pbufl[i] ->next = blistart;
					if(blistart){
						blistart->last = pbufl[i];
					}
					else ASSERT(0);
				}
				else if(i == pbl)
				{
					pbufl[i]->next = pbufl[i-1];//与前面取得联系
					pbufl[i-1]->last = pbufl[i];
					if(blia)
					{
						pbufl[i]->last = blia;
						blia->next = pbufl[i];
					}
					else
					{
						lcnbl->bli = pbufl[i];
					}
				}
				else 
				{
					pbufl[i]->next = pbufl[i-1];
					pbufl[i-1]->last = pbufl[i];
				}
			}
		}
	}
	return ;
}
//for the unexpected dealing with the merge processing 
void vregendercnblunexpectedr(PVCNBL *rcnblin,PVCNBL lcnbl,PVBLI blistart,PVBLI bliend,
					PVBLI pbufr[],int pbr,PVBLI *bliout)
{
	PVCNBL rcnbl = *rcnblin;
	PVBLI  blia  = NULL;
	int i ;
	if(rcnbl->flag == 1)
	{
		if(pbr == 0)
		{
			if(blistart == NULL)
			{
				rcnbl->bli = pbufr[0];
				ASSERT(0);
			}
			else
			{
				pbufr[0]->last = blistart;
				blistart->next = pbufr[0];
			}
		}
		for(i = 0;i<= pbr;i++)
		{
			if(i == 0)
			{
				if(blistart == NULL)
				{
					rcnbl->bli = pbufr[i];
					ASSERT(0);
				}
				else
				{
					pbufr[i]->last = blistart;
					blistart->next = pbufr[i];
				}
			}
			else if (i == pbr)
			{
				pbufr[i]->last = pbufr[i-1];//
				pbufr[i-1]->next = pbufr[i];//
			}
			else 
			{
				pbufr[i]->last = pbufr[i-1];
				pbufr[i-1]->next = pbufr[i];
			}
		}//return;
	}
	else
	{
		//above is the finding of the first/(last in sequential meaning) bli
		blia = vfindimlimitedbli(blistart,2);
		if(pbr == 0)
		{
			if(blistart == NULL)
			{
				rcnbl->bli = pbufr[0];
				if(!blia){
					pbufr[0]->next = blistart->next; 
					ASSERT(0);
					blistart->next->last = pbufr[0];
				}//it never happens,if so it must be rewrited
				ASSERT(0);
				return;
			}
			else
			{
				pbufr[0]->last = blistart;//have been fixed 6-15p.m. 
				blistart->next = pbufr[0];
				pbufr[0]->next = blia;
				blia->last = pbufr[0];
			}
		}
		else{
			for( i = 0; i<=pbr; i++)
			{
				if(i == 0)
				{
					if(blistart == NULL)
					{
						rcnbl->bli = pbufr[i];
						ASSERT(0);//it never happens(?)6-15PM
					}
					else
					{
						blistart->next = pbufr[i];
						pbufr[i]->last = blistart;
					}
				}
				else if(i == pbr)
				{
					pbufr[i]->last = pbufr[i-1];
					pbufr[i-1]->next = pbufr[i];//
					if(blia)
					{
						pbufr[i]->next = blia;
						blia->last = pbufr[i];
					}
				}
				else 
				{
					pbufr[i]->last = pbufr[i-1];
					pbufr[i-1]->next = pbufr[i];
				}
			}
		}
	}
	return ;
}
//找到最近的一个交点，t/大小,blit/所处角平分线连的位子,lrflag/交于那边;
//bns input tfirst,mfirst,and out put tend and mend,
//startflag input the start place of 
//bisector,blil,blir out put... ,irflag out put....
//blil input the first lcnbl bli ,the blir input the first rcnbl
//blil and blif output the bli have been update according to the startflag
//lrflag out put the first intersect bli of cnbls.
//lrflag = 1 intersect first with lcnbl blil be regenerated
//lrflag = 2 rcnbl be regenerated
//lrflag = 0 all blir and blil be regenerated;
//lrflag = -1 no firstpoint been found;  
void vfindfirstpoint(PVCNBL rcnbl,PVCNBL lcnbl,PVBNS bns,int startflag,int directionflag,//input//startflag useless
					 PVBLI *blil,PVBLI *blir,int *lrflag)//output
					 // bli
{
	ASSERT(rcnbl);ASSERT(lcnbl);
	ASSERT(rcnbl->bli);ASSERT(lcnbl->bli);
	//all are template useing parameters
	double tl,tr,tfirst;
	int    ml,mr,flagl,flagr;
	int    mfirst;
	double tmfirst,tml,tmr;
	//
	PVBLI blitl,blitr;
	//
	tfirst = bns->t1;
	//I think this is the wrong statement
	mfirst  = bns->m1;//bns have been the bisector of the validate bisector of lcnbl and rcnbl
	tmfirst = tfirst * mfirst; 
	//
	if(rcnbl->flag==0 && lcnbl->flag==0)
	{
		//this function are writed
		flagl = vgainintersectortl(lcnbl,rcnbl->cns,bns,startflag,directionflag,*blil,&tl,&ml,&blitl);//函数体中得保证有解（唯一）
		flagr = vgainintersectortr(rcnbl,lcnbl->cns,bns,startflag,directionflag,*blir,&tr,&mr,&blitr);//函数体中得保证有解
		if(0==flagl&&0==flagr)
		{
			*lrflag = -1;
			return ;
		}
		else if (0==flagl&&flagr)
		{
			*blir = blitr;
			bns->t2 = tr;
			bns->m2 = mr;
			*lrflag = 2;
			return ;
		}
		else if (flagl && 0==flagr)
		{
			*blil = blitl;
			bns->t2 = tl;
			bns->m2 = ml;
			*lrflag = 1;
			return ;
		}
		tml = tl*ml; tmr = tr*mr;
		if(fabs(tmfirst-tml)<ZERO||fabs(tmfirst-tmr)<ZERO)
		{
			ASSERT(0);//
		}
		else if(tmfirst-tml>ZERO && tmfirst-tmr>ZERO)
		{
			if(fabs(tmr-tml)<=ZERO)
			{
				bns->t2 = tl;
				bns->m2 = ml;
				*blil = blitl;
				*blir = blitr;
				*lrflag = 0;
			}
			else if(tmr-tml>ZERO)
			{
				bns->t2 = tr;
				bns->m2 = mr;
				*blir = blitr;
				*lrflag = 2;
			}
			else
			{
				bns->t2 = tl;
				bns->m2 = ml;
				*blil = blitl;
				*lrflag = 1;
			}
		}
		else if(tmfirst-tml<-ZERO && tmfirst-tmr<-ZERO)
		{
			if(fabs(tmr-tml)<=ZERO)
			{
				bns->t2 = tl;
				bns->m2 = ml;
				*blil = blitl;
				*blir = blitr;
				*lrflag = 0;
			}
			else if(tmr-tml>ZERO)
			{
				bns->t2 = tl;
				bns->m2 = ml;
				*blil = blitl;
				*lrflag = 1;
			}
			else
			{
				bns->t2 = tr;
				bns->m2 = mr;
				*blir = blitr;
				*lrflag = 2;
			}
		}
		else if((tmfirst-tml)*(tmfirst-tmr)<-ZERO)
		{
			if((tmfirst>=0 && tml>=0 )||( tmfirst<0 && tml<0 ))
			{
				bns->t2 = tl;
				bns->m2 = ml;
				*blil = blitl;
				*lrflag = 1;
			}
			else if((tmfirst>=0 && tmr>=0 )||( tmfirst<0 && tmr<0 ))
			{
				bns->t2 = tr;
				bns->m2 = mr;
				*blir = blitr;
				*lrflag =2;
			}
			else
				ASSERT(0);//tfirst 在两者之间
		}
	}
	else if(lcnbl->flag==1 && rcnbl->flag==0)
	{
		//如果flag==0应该有解
		flagl = vgainintersectortr(rcnbl,lcnbl->cns,bns,startflag,directionflag,*blir,&tr,&mr,&blitr);//函数体中得保证有解
		if(flagl == 0)
		{
			//AfxMessageBox("bisector meet with nothing ");
			*lrflag = -1;
			return;
		}
		tmr = tr*mr;
		if(fabs(tmfirst-tmr)<ZERO)
			ASSERT(0);
		bns->t2 = tr;
		bns->m2 = mr;
		*blir = blitr;
		*lrflag = 2;
	}
	else if(lcnbl->flag==0 && rcnbl->flag==1)
	{//why I put the blil into blir
		flagr = vgainintersectortl(lcnbl,rcnbl->cns,bns,startflag,directionflag,*blil,&tl,&ml,&blitl);//函数体中得保证有解
		if(flagr == 0)
		{
			//AfxMessageBox("bisector meet with nothing");
			*lrflag = -1;
			return ;
		}
		tml = tl*ml;
		if(fabs(tmfirst-tml)<ZERO)
			ASSERT(0);
		bns->t2 = tl;
		bns->m2 = ml;
		*blil = blitl;
		*lrflag = 1;
	}
	else 
	{
		ASSERT(0);
	}
}
//删除lcnbl中的角平分线，加入栈中的角平分线
//lcnbl is the main process-needed objector,blistart&bliend is the limitation of voronoi 
//diagram .pbufl[]and it mark pbl is the collection of all the bisector information need
//be inputed into lcnbl voronoi diagram;
//and we are sure that all bisectors have limitations.
void vregendercnbll(PVCNBL *lcnblin,PVCNBL rcnbl,PVBLI blistart,PVBLI bliend,
					PVBLI pbufl[],int pbl,PVBLI *bliout)
{
	if(pbl <= -1)//if no bli appears
	{
		if(blistart->bni->flag)
			blistart->bni->flag = 0;
		return ;
	}
	//
	ASSERT(*lcnblin);ASSERT(blistart);ASSERT(bliend);ASSERT(rcnbl);
	PVBLI  bli,blit,blia,blis,blie;
	double tm,tm1,tm2;
	double  t1,t2;
	int  mvalue;
	int i,flag1 = 0;
	PVCNBL  lcnbl = *lcnblin ;
	//gain blis;
	if(pbufl[0]->clockwise == CW)
	{
		vgainintersectmvaluebns(blistart->bni->bns,
			pbufl[0]->bni->bns,pbufl[0]->bni->bns->t2,&mvalue);
		t1 = pbufl[0]->bni->bns->t2;
	}
	else
	{
		vgainintersectmvaluebns(blistart->bni->bns,
			pbufl[0]->bni->bns,pbufl[0]->bni->bns->t1,&mvalue);
		t1 = pbufl[0]->bni->bns->t1;
	}
	tm = t1 * mvalue;
	tm1 = blistart->bni->bns->t1 * blistart->bni->bns->m1;
	tm2 = blistart->bni->bns->t2 * blistart->bni->bns->m2;
	//merge terminited,means the lcnbl and rcnbl meet in the place of t=0
	//删去bisector
	if(rcnbl->flag == 1 && lcnbl->flag == 1 && pbufl[pbl]->bni->bns->t2!=0)
	{//make all bli out of in the buf  
		if(pbl == 0)
		{
			pbufl[0]->clockwise *=-1;//I add it 6-15
			lcnbl->bli = pbufl[0];
			pbufl[0]->next = blistart;
			if(blistart)
				blistart->last = pbufl[0];
			else ASSERT(0);
			return;
		}
		for(i = pbl;i>=0;i--)
		{
			pbufl[i]->clockwise *=-1;//I add it 6-15 it cause me a lot of time
			if(i == pbl)
			{
				lcnbl->bli = pbufl[i];
			}
			else if(i == 0)
			{
				pbufl[i]->last = pbufl[i+1];
				pbufl[i+1]->next = pbufl[i];
				pbufl[i]->next = blistart;
				if(blistart)
					blistart->last = pbufl[i];
				else ASSERT(0);
			}
			else
			{
				pbufl[i]->last = pbufl[i+1];
				pbufl[i+1]->next = pbufl[i];
			}
		}
		return ;
	}
	//
	if(rcnbl->flag == 1&&lcnbl->flag == 1 && pbufl[pbl]->bni->bns->t2!=0)
	{
		ASSERT(0);
	}
	if((CCW == blistart->clockwise && fabs(tm-tm2)<=ZERO) ||
		(CW == blistart->clockwise && fabs(tm-tm1)<=ZERO))
	{
		blis = blistart;
	}
	else if((CCW == blistart->clockwise && fabs(tm-tm1)<=ZERO) ||
		(CW == blistart->clockwise && fabs(tm-tm2)<=ZERO))
	{
		blis = blistart->last;
	}
	else if(tm-tm1>ZERO&&tm-tm2<-ZERO||(tm-tm2>ZERO&&tm-tm1<-ZERO))
	{
		blis = blistart->last;
		if(blistart->clockwise == CCW)//modify the original
		{
			blistart->bni->bns->t1 = t1;
			blistart->bni->bns->m1 = mvalue;
		}
		else
		{
			blistart->bni->bns->t2 = t1;
			blistart->bni->bns->m2 = mvalue;
		}
	}
	else ASSERT(0);
	//when the bisector have meet nothing but it isn't wrong so have to be delt with
		if(blistart->bni->flag)
		{
			if(blistart == blis)
			{
				blia = blistart->next;
			}
			else
			{
				blia = blistart;
			}
			vregendercnblunexpectedl(lcnblin,rcnbl,blia,bliend,pbufl,pbl,bliout);
			/////
			blistart->bni->flag = 0;
			return;
		}
	//gain blie
	if(pbufl[pbl]->clockwise ==CW)
	{
		t2 = pbufl[pbl]->bni->bns->t1;
		vgainintersectmvaluebns(bliend->bni->bns,
			pbufl[pbl]->bni->bns,pbufl[pbl]->bni->bns->t1,&mvalue);
	}
	else
	{
		t2 = pbufl[pbl]->bni->bns->t2;
		vgainintersectmvaluebns(bliend->bni->bns,
			pbufl[pbl]->bni->bns,pbufl[pbl]->bni->bns->t2,&mvalue);
	}
	tm = t2 * mvalue;
	tm1 = bliend->bni->bns->t1 * bliend->bni->bns->m1;
	tm2 = bliend->bni->bns->t2 * bliend->bni->bns->m2;
	if((CCW == bliend->clockwise && fabs(tm-tm1) <= ZERO) ||
		(CW == bliend->clockwise && fabs(tm-tm2) <= ZERO))
	{
		blie = bliend;
		flag1 = 1;
	}
	else if((CCW == bliend->clockwise && fabs(tm-tm2)<=ZERO) ||
		(CW == bliend->clockwise && fabs(tm-tm1)<=ZERO))
	{
		blie = bliend->next;
	}
	else if(tm-tm1>ZERO && tm-tm2<-ZERO || (tm-tm2>ZERO && tm-tm1<-ZERO))
	{
		blie = bliend->next;
		if(bliend->clockwise == CCW)
		{
			bliend->bni->bns->t2 = t2;
			bliend->bni->bns->m2 = mvalue;
		}
		else
		{
			bliend->bni->bns->t1 = t2;
			bliend->bni->bns->m1 = mvalue;
		}
	}
	else ASSERT(0);
	//添上bisector from blifirst to bliend
	bli = NULL;
	for(i=0;i<=pbl;i++)
	{
		bli = pbufl[i];
		bli->clockwise = -bli->clockwise;
		//assign clockwise
		//assign link info  ??
		if(pbl == 0)//if there is only one besector in the voronoi diagram
		{
			if(blistart == blis)
			{
				if(blistart->next != NULL)
				{
					bli->next = blistart->next;
					blistart->next->last = bli;
				}
				//
				if(bliend == blie)
				{
					ASSERT(0);//the have the duplicited objector
				}
				else
				{
					bli->last = bliend;
					bliend->next = bli;
				}
			}
			else
			{
				bli->next = blistart;
				blistart->last = bli;
				if(bliend != blie)
				{
					bli->last = bliend;
					bliend->next = bli;
				}
				else if(bliend == blie)
				{
					if(bliend->last !=NULL)
					{
						bli->last = bliend->last;
						bliend->last->next = bli;
					}
					else{
						lcnbl->bli = bli;
					}
				}
			}
		}
		else //at least have two besector;
		{
			if(i==0)//Assign the first 
			{
				if(blistart == blis)
				{
					if(blistart->next != NULL)
					{
						bli->next = blistart->next;
						blistart->next->last = bli;
					}
				}
				else
				{
					bli->next = blistart;
					blistart->last = bli;
				}
				blia = bli;
				bli = NULL;
			}
			//assing the last;
			else if(i==pbl)
			{
				//
				if(bliend != blie)
				{
					bli->last = bliend;
					bliend->next = bli;
				}
				else if(bliend == blie)
				{
					if(bliend->last !=NULL)
					{
						bli->last = bliend->last;
						bliend->last->next = bli;
					}
					else{
						lcnbl->bli = bli;
					}
				}
				//
				bli->next = blia;
				blia->last = bli;
			}
			else//assign the middle objector
			{
				bli->next = blia;
				blia->last = bli;
				blia = bli;
			}
		}
		bli = NULL;
	}
	//before delete the rebundant bisector we must find the next lcnbl first;
		lcnbl = vfindcnbll(bliend->bni->othercns,lcnbl);//
		//
		bli = lcnbl->bli;
		while(bli!=NULL)
		{
			if(bli->bni->bns == bliend->bni->bns)
				break;
			bli = bli->next;
		}
		ASSERT(bli);
		*lcnblin = lcnbl;
		*bliout = bli;
	//delete rebundant bisector;information of bnses is left untouched
	blit = blis;
	if(blis == blie&&blis!=NULL)
	{
		delete blis;
		return;
	}
	else if(blis == bliend || blie == blistart ) 
	{
		return;
	}
	else
	{
		while(blit != blie && blit!=NULL){
			blia = blit->last;
			vdeletebli(blit);//? I do not know whether I can delete all node including bns
			blit = blia;
		}
		if(blit!=NULL)
		{
			if(flag1)
				blit->bni->flag = 1;
			else{
				vdeletebli(blit);
			}
		}
		return;
	}
	//
}//ok
//删除rcnbl中的角平分线，加入栈中的角平分线
void vregendercnblr(PVCNBL *rcnblin,PVCNBL lcnbl,PVBLI blistart,PVBLI bliend,
					PVBLI pbufr[],int pbr,PVBLI *bliout)
{
	if(pbr <= -1) return ;
	ASSERT(*rcnblin);ASSERT(blistart);ASSERT(bliend);ASSERT(lcnbl);
	PVBLI   bli,blit,blia,blis,blie;
	double  tm,tm1,tm2;
	double  t1,t2;
	int     mvalue;
	int     i;
	PVCNBL   rcnbl = *rcnblin;
	//删去bisector
	//gain blis first;
	if(pbufr[0]->clockwise == CW)
	{
		t1 = pbufr[0]->bni->bns->t2;
		vgainintersectmvaluebns(blistart->bni->bns,
			pbufr[0]->bni->bns,pbufr[0]->bni->bns->t2,&mvalue);
	}
	else
	{
		t1 = pbufr[0]->bni->bns->t1;
		vgainintersectmvaluebns(blistart->bni->bns,
			pbufr[0]->bni->bns,pbufr[0]->bni->bns->t1,&mvalue);
	}
	tm  = t1 * mvalue;
	tm1 = blistart->bni->bns->t1 * blistart->bni->bns->m1;
	tm2 = blistart->bni->bns->t2 * blistart->bni->bns->m2;
	if(rcnbl->flag == 1&&lcnbl->flag ==1)
	{
		if(pbr == 0)
		{
			if(blistart)
			{
				blistart->next = pbufr[0];
				pbufr[0]->last = blistart;
			}
			else 
			{
				rcnbl->bli = pbufr[0];
				ASSERT(0);
			}
			return;
		}
		for(i=0;i<=pbr;i++)
		{
			if(i == 0)
			{
				if(blistart)
				{
					blistart->next = pbufr[i];
					pbufr[i]->last = blistart;
				}
				else ASSERT(0);
			}
			else
			{
				pbufr[i]->last = pbufr[i-1];
				pbufr[i-1]->next = pbufr[i];
			}
		}
		return ;
	}
	if((CCW == blistart->clockwise && fabs(tm2-tm)<=ZERO) ||
		(CW == blistart->clockwise && fabs(tm1-tm)<=ZERO))
	{
		blis = blistart->next;
	}
	else if((CCW == blistart->clockwise && fabs(tm1-tm)<=ZERO) ||
		(CW == blistart->clockwise && fabs(tm2-tm)<=ZERO))
	{
		blis = blistart;
	}
	else if(tm-tm1>ZERO&&tm-tm2<-ZERO||(tm-tm2>ZERO&&tm-tm1<-ZERO))
	{
		blis = blistart->next;
		if(blistart->clockwise == CCW)
		{
			blistart->bni->bns->t2 = t1;
			blistart->bni->bns->m2 = mvalue;
		}
		else
		{
			blistart->bni->bns->t1 = t1;
			blistart->bni->bns->m1 = mvalue;
		}
	}
	else ASSERT(0);
	if(blistart->bni->flag)
	{
		if(blistart == blis)
		{
			blia = blistart->last;
		}
		else
		{
			blia = blistart;
		}
		vregendercnblunexpectedr(rcnblin,lcnbl,blia,bliend,pbufr,pbr,bliout);
		/////
		blistart->bni->flag = 0;
		return;
	}
	//gain blie
	if(pbufr[pbr]->clockwise == CW)
	{
		t2 = pbufr[pbr]->bni->bns->t1;
		vgainintersectmvaluebns(bliend->bni->bns,
			pbufr[pbr]->bni->bns,pbufr[pbr]->bni->bns->t1,&mvalue);
	}
	else
	{
		t2 = pbufr[pbr]->bni->bns->t2;
		vgainintersectmvaluebns(bliend->bni->bns,
			pbufr[pbr]->bni->bns,pbufr[pbr]->bni->bns->t2,&mvalue);
	}
	tm = t2 * mvalue;
	tm1 = bliend->bni->bns->t1 * bliend->bni->bns->m1;
	tm2 = bliend->bni->bns->t2 * bliend->bni->bns->m2;
	if((CCW == bliend->clockwise && fabs(tm-tm2)<=ZERO) ||
		(CW == bliend->clockwise && fabs(tm-tm1)<=ZERO))
	{
		blie = bliend;
	}
	else if((CCW == bliend->clockwise && fabs(tm-tm1)<=ZERO) ||
		(CW == bliend->clockwise && fabs(tm-tm2)<=ZERO))
	{
		blie = bliend->last;
	}
	else if(tm-tm1>ZERO&&tm-tm2<-ZERO||(tm-tm2>ZERO&&tm-tm1<-ZERO))
	{
		blie = bliend->last;
		if(bliend->clockwise == CCW)
		{
			bliend->bni->bns->t1 = t2;
			bliend->bni->bns->m1 = mvalue;
		}
		else
		{
			bliend->bni->bns->t2 = t2;
			bliend->bni->bns->m2 = mvalue;
		}
	}
	else ASSERT(0);
	//below is the dealing with the special case.
	//添上bisector from blifirst to bliend
	bli = NULL;
	for(i=0;i<=pbr;i++)
	{
		bli = pbufr[i];
		//assign clockwise
		//assign link info  ??
		if(pbr == 0)//if there is only one besector in the voronoi diagram
		{
			if(blistart == blis)
			{
				if(blistart->last == NULL)
				{
					rcnbl->bli = bli;//bli->last=NULL;bli->next=NULL;
				}
				else
				{
					bli->last = blistart->last;
					blistart->last->next = bli;
				}
				//
				if(bliend == blie)
				{
					ASSERT(0);//the have the duplicited objector
				}
				else
				{
					bli->next = bliend;
					bliend->last = bli;
				}
			}
			else
			{
				bli->last = blistart;
				blistart->next = bli;
				if(bliend != blie)
				{
					bli->next = bliend;
					bliend->last = bli;
				}
				else if(bliend == blie)
				{
					if(bliend->next !=NULL)
					{
						bli->next = bliend->next;
						bliend->next->last = bli;
					}
				}
			}
		}
		else //at least have two besector;
		{
			if(i == 0)//Assign the first 
			{
				if(blistart == blis)
				{
					if(blistart->last == NULL)
					{
						rcnbl->bli = bli;//bli->last=NULL;bli->next=NULL;
					}
					else
					{
						bli->last = blistart->last;
						blistart->last->next = bli;
					}
				}
				else
				{
					bli->last = blistart;
					blistart->next = bli;
				}
				blia = bli;
				bli = NULL;
			}
			//assing the last;
			else if(i == pbr)
			{
				//
				if(bliend != blie)
				{
					bli->next = bliend;
					bliend->last = bli;
				}
				else if(bliend == blie)
				{
					if(bliend->next !=NULL)
					{
						bli->next = bliend->next;
						bliend->next->last = bli;
					}
				}
				//
				bli->last = blia;
				blia->next = bli;
			}
			else//assign the middle objector
			{
				bli->last = blia;
				blia->next = bli;
				blia = bli;
			}
		}
		bli = NULL;
	}
	//before deleter the rebundant bisector we must assignt the rcnble and bliout first;
		rcnbl = vfindcnblr(bliend->bni->othercns,rcnbl);//
		//
		bli = rcnbl->bli;
		while(bli != NULL)
		{
			if(bli->bni->bns == bliend->bni->bns)
				break;
			bli = bli->next;
		}
		ASSERT(bli);
		*rcnblin = rcnbl;
		*bliout = bli;
	//delete rebundant bisector;information of bnses is left untouched
	blit = blis;
	if(blis == blie&&blis!=NULL)
	{
		delete blis;
		return;
	}
	else if( blis == bliend ) 
	{
		return;
	}
	else
	{
		while(blit!=blie && blit!=NULL){
			blia = blit->next;
			vdeletebli(blit);//? I do not know whether I can delete all node including bns
			blit = blia;
		}
		if(blit!=NULL)
		vdeletebli(blit);
		return;
	}
	//
}//ok?
//find intersection t;and the label of bli;
// blif is the first bli of lcnbl (**not rcnbl**)
// bli output the first bli of lcnbl regenerted.
// bns input the the bisector of lcnbl and rcns,because(???)
// the bisector must be gennerated in the first processing
//求得角平分线与左边沃诺图区的交点
//是否需要传入bns?//如果没有交点，返回0；否则返回1
int vgainintersectortl(PVCNBL lcnbl,PVCNS rcns,PVBNS bns,int startflag,int directionflag,PVBLI blif,double *t,int *m,PVBLI *bli)
{
	//如果与一条边有两个"有效"交点，只取一个最近点。
	ASSERT(rcns);ASSERT(lcnbl);
	ASSERT(lcnbl->flag!=1);
	ASSERT( blif );
	//
	PVCNS cns;
	PVBLI blig;
	int i,flag1,flag2;
	double tg[2];
	int   mm[2],mvalue;
	int tmlabel = 0;
	//
	double tmfirst = bns->t1 * bns->m1;//how about the validation of m1?
	//                                 //is m1 belong to bns?looking into it afterwards
	blig = blif;
	cns  = blig->bni->othercns;
/*	
	vgainintersect(lcnbl->cns,rcns,cns,&i,tg);
	if(i == 2)//有可能与同一条角平分线有两个交点???
	{
		return 1;
	}
*/
	blig = blig->last;
	int flag = 1;
	while0( blig && flag)
	{
		cns = blig->bni->othercns;
		vgainintersect(lcnbl->cns,rcns,cns,&i,tg);//????
		if(i == 1)
		{
			if(tg[0] - bns->tmin<-ZERO)
			{
				blig = blig->last;
				continue;
			}
		}
		else if(i==2)
		{
			if(tg[0]-bns->tmin < -ZERO && tg[1] - bns->tmin < -ZERO)
			{
				blig = blig->last;
				continue;
			}
			else if(tg[0] - bns->tmin < -ZERO)
			{
				i = 1;
				tg[0] = tg[1];
			}
			else if(tg[1] - bns->tmin < -ZERO)
			{
				i = 1;
			}
		}
		if(i == 0){
			blig = blig->last;
			continue;
		}
		else if(i == 1)
		{
			vgainintersectmvaluebns(bns,blig->bni->bns,tg[0],1,&mvalue);
			mm[0] = mvalue;
			//
			vgainintersectmvaluebns(blig->bni->bns,bns,tg[0],&mvalue);
			flag1 = vjudgetheinbns(blig->bni->bns,tg[0],mvalue);
			if(flag1 == 0)
			{
				blig = blig->last;
				continue;
			}
			else if(flag1 != 0)
			{
				if((tg[0]*mm[0] - tmfirst)*directionflag>=0)
				{//this statement can confirm t*m is in validative position against tmfirst;
					*bli = blig;
					*t = tg[0];
					*m = mm[0];
					flag = 0;
				}
				else 
				{
					blig = blig->last;
					continue;
				}
			}
		}
		else if(i==2)
		{
			vgainintersectmvaluebns(bns,blig->bni->bns,tg[0],&mvalue);
			mm[0] = mvalue;
			vgainintersectmvaluebns(bns,blig->bni->bns,tg[1],&mvalue);
			mm[1] = mvalue;
			//
			vgainintersectmvaluebns(blig->bni->bns,bns,tg[0],&mvalue);
			flag1 = vjudgetheinbns(blig->bni->bns,tg[0],mvalue);
			//
			vgainintersectmvaluebns(blig->bni->bns,bns,tg[1],&mvalue);
			flag2 = vjudgetheinbns(blig->bni->bns,tg[1],mvalue);
			if(0 == flag1 ) 
			{
				if(0 == flag2)
				{
					blig = blig->last;
					continue;
				}
				else if(0 != flag2)
				{
					if((tg[1]*mm[1] - tmfirst)*directionflag>=0)
					{
						*bli = blig;
						*t = tg[1];
						*m = mm[1];
						flag = 0;
					}
					else
					{
						blig = blig->last;
						continue;
					}
				}
			}
			else if(flag1 != 0 )
			{
				if(0 == flag2)
				{
					if((tg[0]*mm[0] - tmfirst)*directionflag>=0){
						*bli = blig;
						*t = tg[0];
						*m = mm[0];
						flag = 0;
					}
					else {
						blig =blig->last;
						continue;
					}
				}
				else if(0 != flag2)
				{
					double tg1,tg2;
					tg1 = tg[0]*mm[0];
					tg2 = tg[1]*mm[1];
					if((tg1-tmfirst)*directionflag>=0)
					{
						if((tg2-tmfirst)*directionflag>=0)
						{
							if((tg1-tg2)*directionflag>=0)
							{
								tg[0] = tg[1];
						        mm[0] = mm[1];
							}
							/*else
							{tg[0]=tg[0];mm[0]=mm[0];//so it can be left out
							}*/
						}
						/*else 
						{
						}*/
					}
					else 
					{
						if((tg2-tmfirst)*directionflag>=0)
						{
							tg[0] = tg[1];
					        mm[0] = mm[1];
						}
						else 
						{
							blig = blig->last;
							continue;
						}
					}
					*bli = blig;
					*t = tg[0];
					*m = mm[0];
					flag = 0;
				}
				//flag = 0;
			}
		}
		blig = blig->last;
	}
	if(flag == 1){
		//AfxMessageBox("you have counter no intersector,so you have to return and leave the processing to the uper functions");
		return 0;
	}//
	return 1;
}
//求得角平分线与右边沃诺图区的交点
int vgainintersectortr(PVCNBL rcnbl,PVCNS lcns,PVBNS bns,int startflag,int directionflag,PVBLI blif,double *t,int *m,PVBLI *bli)
{
	//如果与一条边有两个交点，只取一个最近点。
	ASSERT(lcns);ASSERT(rcnbl);
	ASSERT(rcnbl->flag!=1);
	ASSERT(blif);
	//
	PVCNS cns;
	PVBLI blig;
	int i,flag1,flag2;
	double tg[2],tmfirst,dtemp;
	int   mm[2],mvalue;
	//
	*bli=NULL;
	tmfirst = bns->m1*bns->t1;
	//
	blig = blif;
	//
	blig = blif->next;
	int flag = 1;
	while( blig && flag)
	{
		cns = blig->bni->othercns;
		vgainintersect(rcnbl->cns,lcns,cns,&i,tg);//
		if(i == 0){
			blig = blig->next;
			continue;
		}
		else if(i == 1)
		{
			dtemp = tg[0];
			flag1 = vjudgetheinbns(blig->bni->bns,dtemp,1);
			flag2 = vjudgetheinbns(blig->bni->bns,dtemp,-1);
			if(flag1 == 0&&flag2 == 0)
			{
				blig = blig->next;
				continue;
			}
			if(flag1&&flag2)
			{
				mvalue = -1;
				vgainintersectmvaluebns(bns,blig->bni->bns,&dtemp,1,&mvalue);
				mm[0] = mvalue;
				vgainintersectmvaluebns(blig->bni->bns,bns,&dtemp,&mvalue);
				flag1 = vjudgetheinbns(blig->bni->bns,dtemp,mvalue);
			}
			else{
				if(flag1)mvalue = 1;
				if(flag2)mvalue = -1;
			}
			dtemp = tg[0];
			vgainintersectmvaluebns(bns,blig->bni->bns,&dtemp,1,&mvalue);
			mm[0] = mvalue;
				vgainintersectmvaluebns(blig->bni->bns,bns,&dtemp,&mvalue);
				flag1 = vjudgetheinbns(blig->bni->bns,dtemp,mvalue);
		}
		else if(i==2)
		{
		}
		blig=blig->next;//this if the only diffirence between vgainbisectortr
		                //and vgainbisectortl;
	}
	if(flag == 1){
		//AfxMessageBox("you have counter no intersector,so you have to return and leave the processing to the uper functions");
		return 0;
	}//
	return 1;
}
//.....................................................................
void vmergetwoprofile(PVPL pl,PVPL pr,PVPL *vpl)
{
	ASSERT(pl);ASSERT(pr);ASSERT(pl->eplf);ASSERT(pl->splf);
	ASSERT(pr->splf);ASSERT(pr->eplf);
	//template used 
	PVCNBL  lcnbl,rcnbl,tlcnbl,trcnbl,cnbl=NULL;
	PVBLI   bli;
	PVBNS   bns = NULL;
	int     clockwiseflag = 1;
	int     directionflag = 0;
	//initial
	int     mfirst = 1;
	int     mblis  = 1;//mblis = ?
	double  tfirst = 0;
	int     startflag = 0;//=0,from l and r,=1 start from l,=2  start from r only
	//
	PVBLI   blir,blil;
	PVBLI   blistr,blistl;
	//flag tab
	double  mtemp;
	int     lrflag;
	//stack
	PVBLI pbufl[40],pbufr[40];//栈
	int   pbl = -1, pbr = -1;
	//
	tlcnbl = pl->splf->scnbl;
	trcnbl = pr->eplf->ecnbl;
	//
	lcnbl = tlcnbl->ccw; //leave the point segment out
	rcnbl = trcnbl->cw;  //leave the point segment out
	lcnbl->cw = rcnbl;
	rcnbl->ccw = lcnbl;
	//vdeletevendpointcnbl(tlcnbl);
	//vdeletevendpointcnbl(trcnbl); //flag=1 mean the pointsegment for ending
	//initial
	blil = lcnbl->bli;
	while(blil->next!=NULL)
	{
		blil = blil->next;
	}
	blir = rcnbl->bli;
	blistl = blil;
	blistr = blir;
	//
	int iflag=1;
	while(iflag)
	{
		if(!(1==rcnbl->flag && 1==lcnbl->flag))// if not all met the end of cnbl node.
		{
			bns = NULL;
			vgainbisector(lcnbl->cns,rcnbl->cns,&bns);//关键的函数,可以是无穷函数		
			//放入角平分线作为参数传入bns不能释放
			if(bns == NULL)     //只有终止时才会有空或,交点到无穷远
			{
				return ; //应该得到一条无限的角平分线,并结束缝合
			}//it is impossible
			else
			{
				bns->t1 = tfirst;//I think/wonder the statement can be delete safely 
				if( pbl == -1  )//如果两边都空或左空
				{
					vgainintersectmvaluebns(bns,blistl->bni->bns,&tfirst,mblis,&mfirst);
					bns->m1 = mfirst;
					bns->t1 = tfirst;
					directionflag = vfinddirection(blil,bns,tfirst,mfirst);
				}
				else if(pbr == -1 )//右空
				{
					vgainintersectmvaluebns(bns,blistr->bni->bns,&tfirst,mblis,&mfirst);
					bns->m1 = mfirst;
					bns->t1 = tfirst;
					directionflag = vfinddirection(blir,bns,tfirst,mfirst);
				}
			}
			//give the vfindfirstpoint a direction
				ASSERT(directionflag);
			//
			//找到最近的一个交点，t/大小,blit/所处角平分线连的位子,lrflag/交于那边;
			vfindfirstpoint(rcnbl,lcnbl,bns,startflag,directionflag,//VIF(very impotant function)
				&blil,&blir ,&lrflag );//i=0 none intersect,i=1 left,i=2,right;
			//对tfirst进行赋值运算//可行
			if(lrflag == -1)//there aren't any intersect point meet the needs
			{//应有特殊处理//modified in 6-17
				bli = vmallocbli();
				bli->bni = vmallocbni();
				bli->bni->bns = bns;
				if(directionflag == CCW)
				{
					bli->bni->bns->m2 = 1;
					clockwiseflag = 1;
				}
				else if(directionflag == CW)
				{
					bli->bni->bns->m2 = -1;
					clockwiseflag = -1;
				}
				//
				pbl++;
				pbufl[pbl] = bli;
				bli->bni->selfcns = lcnbl->cns;
				bli->bni->othercns = rcnbl->cns;
				bli->clockwise = clockwiseflag;
				bli = NULL;
				//
				bli = vmallocbli();
				bli->bni = vmallocbni();
				bli->bni->bns = bns;
				pbr++;
				pbufr[pbr] = bli;
				bli->bni->selfcns = rcnbl->cns;
				bli->bni->othercns = lcnbl->cns;
				bli->clockwise = clockwiseflag;
				bli = NULL;
				//
				*vpl = pl;
				pl->ptype = VCOMMON;
				blistr->bni->flag = 1;
				blistl->bni->flag = 1;//bni->flag have left for the special processing
				vregendercnblr(&rcnbl,lcnbl,blistr,blir,pbufr,pbr,&bli);//
				vregendercnbll(&lcnbl,rcnbl,blistl,blil,pbufl,pbl,&bli);//
				pl->splf->scnbl = pr->splf->scnbl; 
				pl->eplf->ecnbl = pl->eplf->ecnbl;
				return;
			}
			//
			tfirst = bns->t2;
			//在此之前，bns还没有被排列，即bli中的clockwise没有被更新
			bli = vmallocbli();
			bli->bni =vmallocbni();
			bli->bni->bns = bns;
			clockwiseflag = vgetclockwisebli(bli);
			//
			//
			pbl++;
			pbufl[pbl] = bli;
			bli->bni->selfcns = lcnbl->cns;
			bli->bni->othercns = rcnbl->cns;
			bli->clockwise = clockwiseflag;
			bli = NULL;
			//
			bli = vmallocbli();
			bli->bni = vmallocbni();
			bli->bni->bns = bns;
			pbr++;
			pbufr[pbr] = bli;
			bli->bni->selfcns = rcnbl->cns;
			bli->bni->othercns = lcnbl->cns;
			bli->clockwise = clockwiseflag;
			bli = NULL;
			//bns = NULL;//in this time the bns can't be put zero
			//压栈
			//
			if(lrflag == 1)//先与左边相交
			{
				//删除lcnbl中的角平分线，加入栈中的角平分线
				vregendercnbll(&lcnbl,rcnbl,blistl,blil,pbufl,pbl,&bli);//?
				pbl=-1;//清空栈
				//找到blil的另一部分cns对应的cnbl节点
				startflag = 1;
				//
				blil = blistl = bli;
				mblis = bli->bni->bns->m2;
				bli = NULL;
			}   
			else if(lrflag == 2) //intersect with right
			{
				vregendercnblr(&rcnbl,lcnbl,blistr,blir,pbufr,pbr,&bli);//?
				pbr = -1;//清空栈
				//清空并更新
				startflag = 2;
				//
				blir = blistr = bli;
				mblis = bli->bni->bns->m1;
			}
			else if(lrflag == 0)//intersect with two all simultaniously
			{
				cnbl = rcnbl;
				vregendercnblr(&rcnbl,lcnbl,blistr,blir,pbufr,pbr,&bli);//
				blir = blistr = bli;
				bli  = NULL;
				//
				vregendercnbll(&lcnbl,cnbl,blistl,blil,pbufl,pbl,&bli);//
				blil = blistl = bli;
				mblis = bli->bni->bns->m2;
				bli = NULL;
				//
				pbr=-1;pbl=-1;//清空栈
				//请空并更新
				startflag = 0;
			}
		}
		//
		else if(1==rcnbl->flag&&1==lcnbl->flag)//both left and right  end
		{
			*vpl = pl;
			pl->ptype = VCOMMON;
			if(tfirst<=ZERO)//this is the end of the whole merge processing.
			{
				tlcnbl = lcnbl->cw;
				trcnbl = rcnbl->ccw;
				tlcnbl->ccw = trcnbl;
				trcnbl->cw  = tlcnbl;
				vdeletevendpointcnbl(lcnbl);
				vdeletevendpointcnbl(rcnbl);
				pl->splf->scnbl = trcnbl;
				pl->eplf->ecnbl = tlcnbl;
			}
			else
			{
				vregendercnblr(&rcnbl,lcnbl,blistr,blir,pbufr,pbr,&bli);//
				vregendercnbll(&lcnbl,rcnbl,blistl,blil,pbufl,pbl,&bli);//
				pbr=-1;pbl=-1;//清空栈
				pl->splf->scnbl = pr->splf->scnbl; 
				pl->eplf->ecnbl = pl->eplf->ecnbl;
			}
			return;
		}
    }
}
//计算卧诺图
void  vmergesingleprofile(PVPL profile)
{
	ASSERT(profile);
	PVPL pl,pr,midprofile;
	PVBNS bns;
	PVCNBL cnbl;
	PVBLI bli;
	int i = 0;

	if(SINGLE == profile->ptype)//判断是否是叶子/单树枝ok
	{
		vgainsingleplfbli(profile->eplf);//给叶子赋值//ok perhaps
		if(0)
		{
			cnbl = profile->eplf->scnbl;
			do{
				bli = cnbl->bli;
				do{
					vdisplaysinglebi(*bli->bni->bns,RGB(0,0,255));
					AfxMessageBox("show the result");
					bli = bli->next;
				}while(bli);
				cnbl = cnbl->ccw;
			}while(cnbl!=profile->eplf->ecnbl);
		}
		return;
	}
	vdivideprofiletwo(profile,&pl,&pr);//均分树枝；//ok
	vmergesingleprofile(pl);
	if(i)
	{
		vdisplayplv(*pl);
		AfxMessageBox("show the result");
	}
	vmergesingleprofile(pr);
	if(i)
	{
		vdisplayplv(*pr);
		AfxMessageBox("show the result");
	}
	vmergetwoprofile(pl,pr,&midprofile);//缝合枝枝叶叶waiting
	myflag++;
	if(i)
	{
		vdisplayplv(*midprofile);
		AfxMessageBox("show the result");
	}
	profile->eplf  = midprofile->eplf ;
	profile->splf  = midprofile->splf ;
	profile->ptype = midprofile->ptype;
}
/*the end*/