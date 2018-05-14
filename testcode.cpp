#include "testcode.h"
/*
Testcode::Testcode()
{
}

Testcode::~Testcode()
{
}
*/
/*

1．  求出图象的最大灰度值和最小灰度值，分别记为ZMAX和ZMIN，令初始阈值T0=(ZMAX+ZMIN)/2；

2．  根据阈值TK将图象分割为前景和背景，分别求出两者的平均灰度值ZO和ZB；

3．  求出新阈值TK+1=(ZO+ZB)/2；

4．  若TK=TK+1，则所得即为阈值；否则转2，迭代计算。

*/
int Testcode::AdaptiveThreshold(int t, IplImage *Image)  //这个为什么要这样做 使用这种方式 原因不清楚
{
    printf("AdaptiveThreshold");
        int t1=0,t2=0,tnew=0,i=0,j=0;
        int Allt1=0,Allt2=0,accountt1=0,accountt2=0;//Allt1 Allt2 保存两部分的和
        for(j=0;j<Image->height;j++) //根据现有T，将图像分为两部分，分别求两部分的平均值t1、t2
        {
            for(i=0;i<Image->width;i++)
            {
                if(CV_IMAGE_ELEM(Image,uchar,j,i)<t)
                {
                    Allt1+=CV_IMAGE_ELEM(Image,uchar,j,i);
                    accountt1++;
                }
                else
                {
                    Allt2+=CV_IMAGE_ELEM(Image,uchar,j,i);
                    accountt2++;
                }
            }
        }
        t1=Allt1/accountt1;
        t2=Allt2/accountt2;
        tnew=0.5*(t1+t2);
        if(tnew==t) //若t1、t2的平均值和t相等，则阈值确定
            return tnew;
        else
            AdaptiveThreshold(tnew,Image); //不等则以t1、t2的平均值为新阈值迭代
}


void Testcode::Threshold(IplImage *Image, IplImage *Image_O)
{
    printf("Threshold");
        //得到图片的最大灰度值和最小灰度值
        int thresMax=0,thresMin=255,i=0,j=0,t=0;
        for(j=0;j<Image->height;j++)
            for(i=0;i<Image->width;i++)
            {
                if(CV_IMAGE_ELEM(Image,uchar,j,i)>thresMax) //像素值 大于 255
                    thresMax=CV_IMAGE_ELEM(Image,uchar,j,i);  //把元素值赋给 thresMax
                else if(CV_IMAGE_ELEM(Image,uchar,j,i)<thresMin) //如果小于 0
                    thresMin=CV_IMAGE_ELEM(Image,uchar,j,i); //则 改变thresMin
            }
            //int T=(thresMax+thresMin)*0.5; //灰度的最大值和最小值的平均
            cvCanny(Image,Image_O,AdaptiveThreshold((thresMax+thresMin)*0.5,Image),thresMax*0.7,3);
            //小阈值用来控制边缘连接  大阈值用来控制强边缘的初始化分割  cvCanny只接受单通道的输入
}

int Testcode::PlateAreaSearch(IplImage *pImg_Image)
    {
        printf("PlateAreaSearch");
        if (pImg_Image==NULL)	{ return 0;	} // 检测是否有值

        IplImage* imgTest =0;
        int i=0, j=0,k=0,m=0;
        bool flag=0;
        int plate_n=0 ,plate_s=0,plate_e=0 ,plate_w=0;  //关于车牌的一些变量

        int *num_h=new int[max(pImg_Image->width,pImg_Image->height)];
        if ( num_h==NULL )
        {
            //cout<<"memory exhausted"<<endl;

            return 0;
            //	exit(1);
        }  // end if

        for(i=0;i<pImg_Image->width;i++){num_h[i]=0;}  // 初始化 分配的空间

        imgTest = cvCreateImage(cvSize(pImg_Image->width,pImg_Image->height),IPL_DEPTH_8U,1);
        cvCopy(pImg_Image, imgTest);

        //-- 水平 轮廓细化
        for(j=0; j<imgTest->height; j++)
        {
            for(i=0;i<imgTest->width-1;i++)
            {
                CV_IMAGE_ELEM(imgTest,uchar,j,i)=CV_IMAGE_ELEM(imgTest,uchar,j,i+1)-CV_IMAGE_ELEM(imgTest,uchar,j,i);
                num_h[j]+=CV_IMAGE_ELEM(imgTest,uchar,j,i)/250;
            }
        }


        int temp_1=0;
        int temp_max =0;
        int temp_i = 0;
        //说明这里for 循环 是找出数据量最大的地方  20行  也即是 车牌区域
        for(j=0; j<imgTest->height-20; j++)
        {
            temp_1=0;
            for(i=0;i<20;i++)//此处for循环 是为了计算20行的总数据量
                temp_1 +=  num_h[i+j];
            if(temp_1>=temp_max)
            {
                temp_max=temp_1;
                temp_i = j;//记录20行的最大数据量的开始 行
            }
        }
        k=temp_i;//以下两个while 循环是为了找出 车牌的上下边界  当一行的数据量小于某个数值时 设定此为分界行
        while ( ((num_h[k +1]>POINT_X )||(num_h[k +2]>POINT_X )||(num_h[k]>POINT_X )) && k ) k--;//出行边界行
        plate_n=k+1;//k+2;
        k=temp_i+10;
        while (((num_h[k -1]>POINT_X )||(num_h[k-2]>POINT_X )||(num_h[k]>POINT_X ))&&(k<imgTest->height)) k++; //出下边界行
        plate_s=k;//k-2;

        // 没找到水平分割线，设置为默认值
        if ( !(plate_n && plate_s  //行为负值 或者 上行大于下行  或者 车牌宽度大于 设定值 则水平分割失败
            && (plate_n<plate_s) && ((plate_s-plate_n)*HIGH_WITH_CAR<imgTest->width*(1-WITH_X))))
        {
            //flag=1;
            //	cout<<"水平分割失败"<<endl;

            return 0;
        }
        else//到水平线
        {
            int  max_count = 0;
            //这里 plate_length  的值是 原图像的宽度减去车牌的宽度差值。
            int  plate_length = (imgTest->width-(plate_s-plate_n)*HIGH_WITH_CAR);
            plate_w=imgTest->width*WITH_X-1;//车牌左边界 设置默认值

            //--垂直方向
            for(i=0;i<imgTest->width;i++)
                for(j=0;j<imgTest->height-1;j++)//用的方法是 差分赋值法 为了细化
                {
                    CV_IMAGE_ELEM(imgTest,uchar,j,i)=CV_IMAGE_ELEM(imgTest,uchar,j+1,i)-CV_IMAGE_ELEM(imgTest,uchar,j,i);
                }
                //下面这一段代码 相当于 拿一个车牌大小的矩形区域 从左往右 滑动，什么时候圈住的数据量最大的时候
                // 就代表找到的车牌的左边界 此时 车牌左边界的横坐标是 k
                //这里 plate_length  的值是 原图像的宽度减去车牌的宽度差值。
                for(k=0;k<plate_length;k++)
                {
                    for(i=0; i<(int)((plate_s-plate_n)*HIGH_WITH_CAR); i++)
                        for (j=plate_n;j<plate_s;j++)//两水平线之间
                        {
                            num_h[k] =num_h[k]+ CV_IMAGE_ELEM(imgTest,uchar,j,(i+k))/250;
                        }
                        if (num_h[k]>max_count)
                        {
                            max_count = num_h[k];
                            plate_w = k;
                        }  // end if

                }

                CvRect ROI_rect;                 //获得图片感兴趣区域
                ROI_rect.x=plate_w;
                ROI_rect.y=plate_n;
                ROI_rect.width=(plate_s-plate_n)*HIGH_WITH_CAR;
                ROI_rect.height=plate_s-plate_n;

                if ((ROI_rect.width+ROI_rect.x)> pImg_Image->width)//图像越界
                {
                    ROI_rect.width=pImg_Image->width-ROI_rect.x;
                    //	cout<<"垂直方向分割失败！";

                    return 0;
                }
                else
                {

                    IplImage *pImg8uROI=NULL;         //感兴趣的图片
                    pImg8uROI=cvCreateImage(cvSize(ROI_rect.width,ROI_rect.height), src->depth,src->nChannels);

                    IplImage *pImg8u11=NULL;        //车牌区域插值后的图
                    pImg8u11=cvCreateImage(cvSize(40*HIGH_WITH_CAR,40),pImg8uROI->depth,pImg8uROI->nChannels);

                    cvSetImageROI(src,ROI_rect);//将src中的 ROI_rect 设置为感兴趣区域
                    cvCopy(src,pImg8uROI,NULL);//把感兴趣区域 复制到 pImg8uROI
                    cvResetImageROI(src);   //释放src感兴趣区域

                    pImgResize=cvCreateImage(cvSize(40*HIGH_WITH_CAR,40),IPL_DEPTH_8U,1);
                    cvResize(pImg8uROI,pImg8u11,CV_INTER_LINEAR); //线性插值  归一化  把车牌变成统一大小

                    cvCvtColor(pImg8u11,pImgResize,CV_RGB2GRAY);  //  转为灰度图	  Y=0.299*R + 0.587*G + 0.114*B
                    Threshold(pImgResize,pImgResize);             // 二值化

                    //cvNamedWindow("ccc", 1);
                    //cvShowImage("ccc", pImgResize);//显示车牌

                    cvReleaseImage(&pImg8uROI);
                    cvReleaseImage(&pImg8u11);
                    cvReleaseImage(&imgTest);
                }   // end if
        }    // end if

        // 释放内存
        delete []num_h;
        num_h=NULL;
        return 1;
}

int Testcode::SegmentPlate()
{
        printf("SegmentPlate\n");
        if (pImgResize==NULL)  { return 0; } // 没有切割成功，直接弹出

        int *num_h=new int[max(pImgResize->width,pImgResize->height)];	 // 开辟空间  一般应该是 width 大小
        if ( num_h==NULL )   //
        {
            cout<<"memory exhausted"<<endl;
            //MessageBox("字符分割memory exhausted");
            return 0;
            //exit(1);
        }  // end if
        int i=0,j=0,k=0;//循环变量 12
        int  letter[14]={0,20,23,43,55,75,78,98,101,121,124,127,147,167}; // 默认分割
        bool flag1=0;//    1     2     3     4       5           6    7
        // 垂直投影
        //计算每一列数值和 只用0-17和24-40因为数据量相对小，大致定位就好，计算快
        for(i=0;i<40*HIGH_WITH_CAR;i++)
        {
            num_h[i]=0; // 初始化指针
            for(j=0;j<17;j++)  // 0-16 /40
            {
                num_h[i]+=CV_IMAGE_ELEM(pImgResize,uchar,j,i)/45;

            }
            for(j=24;j<40;j++)  // 24-39 /40
            {
                num_h[i]+=CV_IMAGE_ELEM(pImgResize,uchar,j,i)/45;
            }
            //cout << i << "num" << num_h[i] << endl;//调试
        }
        // 初定位，定位点 第二个字符末端，
        int	max_count=0;
        int   flag=0;
        for(i=30;i<40*HIGH_WITH_CAR;i++)
        {
            if(num_h[i]<POINT_Y)//小于2 说明查到了第二个字符和第三个字符之间的空隙
            {
                max_count++;
                if(max_count==11)
                {
                    letter[3]=i-11; // find letter[3]//第二字符的结束位置
                    while( (num_h[i]<POINT_Y)||(num_h[i-1]<POINT_Y) ) i++;
                    letter[4]=i-1; // find letter[4]  //第三个字符的开始位置
                    break;//只要找到 第二个字符的末端 和 第三个字符的开始 就退出循环
                }
            }
            else
            {
                max_count=0;
            }
        }
        // 精定位
        for(i=0;i<40*HIGH_WITH_CAR;i++)//每一列的
        {
            for(j=17;j<=24;j++)  // 17-24 /40   每一列的17 到 24 行相加 数据量大一点
            {
                num_h[i]+=CV_IMAGE_ELEM(pImgResize,uchar,j,i)/45;
            }
        }

        for(j=letter[3];j>0;j--)//从第二个字符的末端开始 往前找第一个和第二个字符起始位置
        {
            if((num_h[j]<POINT_Y)&&(num_h[j-1]<POINT_Y))//只要有两个列的 17到24 行的值小于 2，
            {                                           //即找到第二个字符的开始位置
                letter[2]=j;      // find letter[2]  第二个字符的开始位置
                letter[1]=(j>=23)?j-3:letter[1];   //第一个字符的结束位置
                letter[0]=(j>=23)?j-23:letter[0];  //第一个字符的起始位置
                break;       //找到就退出循环
            }
        }

        j=2;  flag=0;flag1=0;//两个标记
        for(i=letter[4];i<40*HIGH_WITH_CAR;i++)  //从第三个字符的开始位置算起
        {
            if((num_h[i]>POINT_Y)&&(num_h[i-1]>POINT_Y) && !flag )
            {
                flag=1;
                flag1=0;
                letter[2*j]=i-1; //这里 只记录字符的开始位置
                if(j==6)  //判断 最后一个字符的结束位置 是否越界 超出界限,如果没有,则letter[13]=letter[12]+20
                {
                    letter[2*j+1]=((letter[2*j]+20)>40*HIGH_WITH_CAR-1)?40*HIGH_WITH_CAR-1:letter[2*j]+20;
                    break;//退出 for循环
                }
            }
            else if((num_h[i]<POINT_Y)&&(num_h[i-1]<POINT_Y) && !flag1 && flag)//如果是 空白区域
            {
                flag=0;
                flag1=1;
                letter[2*j+1]=i-1;
                j++; //j自动加 1
            }                        //    1
        }
        // 删除角点                              1   0   1
        for(i=0;i<40*HIGH_WITH_CAR-1;i++)
        {	                               //      1    删除角点  相当于拿一个半径为1 的圆 去圈 如果四周有两个是1  则自己设置为0

            for(j=0;j<39;j++)  // 0-16 /40
            {

                if(CV_IMAGE_ELEM(pImgResize,uchar,j,i)&&CV_IMAGE_ELEM(pImgResize,uchar,j,i+1)&&CV_IMAGE_ELEM(pImgResize,uchar,j+1,i)) //  01
                    CV_IMAGE_ELEM(pImgResize,uchar,j,i)=0;                                                                                                                                                                                                                             //   1

                if(CV_IMAGE_ELEM(pImgResize,uchar,j,i)&&  CV_IMAGE_ELEM(pImgResize,uchar,j,i-1) &&CV_IMAGE_ELEM(pImgResize,uchar,j+1,i))     //  10
                    CV_IMAGE_ELEM(pImgResize,uchar,j,i)=0;                                                                                                                                                                                                                               //    1

                if(CV_IMAGE_ELEM(pImgResize,uchar,j,i)&&CV_IMAGE_ELEM(pImgResize,uchar,j,i-1) &&CV_IMAGE_ELEM(pImgResize,uchar,j-1,i))         //   1
                    CV_IMAGE_ELEM(pImgResize,uchar,j,i)=0;                                                                                    // 10
                if(CV_IMAGE_ELEM(pImgResize,uchar,j,i)&&CV_IMAGE_ELEM(pImgResize,uchar,j,i+1) &&CV_IMAGE_ELEM(pImgResize,uchar,j-1,i))           // 1
                    CV_IMAGE_ELEM(pImgResize,uchar,j,i)=0;                                                                                 // 01
            }
        }

        // 分割出字符图片
        pImgCharOne=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);
        pImgCharTwo=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);
        pImgCharThree=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);
        pImgCharFour=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);
        pImgCharFive=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);
        pImgCharSix=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);
        pImgCharSeven=cvCreateImage(cvSize(20,40),IPL_DEPTH_8U,1);

        CvRect ROI_rect1;
        ROI_rect1.x=0.5*(letter[1]+letter[0])-10; //为什么减10  你知道吗~~~  这个不难
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharOne,NULL); //获取第1个字符
        cvResetImageROI(pImgResize);

        ROI_rect1.x=0.5*(letter[3]+letter[2])-10;
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharTwo,NULL); //获取第2个字符
        cvResetImageROI(pImgResize);

        ROI_rect1.x=0.5*(letter[5]+letter[4])-10;
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharThree,NULL); //获取第3个字符
        cvResetImageROI(pImgResize);

        ROI_rect1.x=0.5*(letter[7]+letter[6])-10;
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharFour,NULL); //获取第4个字符
        cvResetImageROI(pImgResize);

        ROI_rect1.x=0.5*(letter[9]+letter[8])-10;
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharFive,NULL); //获取第5个字符
        cvResetImageROI(pImgResize);

        ROI_rect1.x=0.5*(letter[11]+letter[10])-10;
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharSix,NULL); //获取第6个字符
        cvResetImageROI(pImgResize);

        ROI_rect1.x=0.5*(letter[13]+letter[12])-10;
        ROI_rect1.y=0;
        ROI_rect1.width=20;
        ROI_rect1.height=40;
        cvSetImageROI(pImgResize,ROI_rect1);
        cvCopy(pImgResize,pImgCharSeven,NULL); //获取第7个字符
        cvResetImageROI(pImgResize);
        // 释放内存
        delete []num_h;
        num_h=NULL;
    }



int Testcode::CodeRecognize(IplImage *imgTest, int num, int char_num)
{
      printf("CodeRecognize");
        if (imgTest==NULL){ return 0;}

        int i=0,j=0,k=0,t=0;//循环变量
        int  char_start=0,char_end=0;//*PlateCode[TEMPLETENUM] 车牌字符里字母、数字、汉字起始位置
        int num_t[CHARACTER ]={0};


        switch(num)//这里这样分 可以提高效率，并且提高了识别率
        {
        case 0:  char_start =0;         // 数字
            char_end  = 9;
            break;
        case 1:  char_start =10;        // 英文
            char_end  = 35;
            break;
        case 2:  char_start =0;       // 英文和数字
            char_end  = 35;
            break;
        case 3:  char_start =36;       // 中文
            char_end  = TEMPLETENUM-1;
            break;
        default: break;
        }

        // 提取前8个特征  前8个特征 可以说是固定位置的值  固定算法
        for(k=0; k<8; k++)
        {
            for(j=int(k/2)*10; j<int(k/2+1)*10; j++)
            {
                for(i=(k%2)*10;i<(k%2+1)*10;i++)
                {
                    num_t[k]+=CV_IMAGE_ELEM(imgTest,uchar,j,i)/255 ;
                }
            }
            num_t[8]+= num_t[k];  // 第9个特征 前8个特征的和作为第9个特征值
        }

        for(i=0;i<20;i++)  //以下特征也是 固定算法得到的
            num_t[9]+=CV_IMAGE_ELEM(imgTest,uchar,10,i)/255 ;
        for(i=0;i<20;i++)
            num_t[10]+=CV_IMAGE_ELEM(imgTest,uchar,20,i)/255 ;
        for(i=0;i<20;i++)
            num_t[11]+=CV_IMAGE_ELEM(imgTest,uchar,30,i)/255 ;

        for(j=0;j<40;j++)
            num_t[12]+=CV_IMAGE_ELEM(imgTest,uchar,j,7)/255;
        for(j=0;j<40;j++)
            num_t[13]+=CV_IMAGE_ELEM(imgTest,uchar,j,10)/255 ;
        for(j=0;j<40;j++)
            num_t[14]+=CV_IMAGE_ELEM(imgTest,uchar,j,13)/255 ;

        int num_tt[CHARACTER]={0};
        int matchnum=0;  //可以说是 匹配度或 相似度
        int matchnum_max=0;
        int matchcode = 0;         // 匹配号
        //int matchtempnum[10]={0};

        j=0;

        for(k=char_start;k<=char_end;k++)
            //for(k=40;k<42;k++)
        {
            matchnum=0;

            for(i=0;i<8;i++) //区域的匹配
            {
                //	num_tt[i]= abs(num_t[i]-num[k][i]);
                if (abs(num_t[i]-Num_Templete[k][i])<=2)//与模板里的相应值进行匹配
                    matchnum++;//两者相减，如果绝对值小于2，标记匹配成功一次
            }

            if(Num_Templete[k][i]-abs(num_t[i])<=8)//对第9个特征进行匹配
                matchnum+=2;
            for(i=9;i<CHARACTER;i++)  // 横竖的匹配
            {
                if (Num_Templete[k][i]>=5)  //特征值 大于5
                {
                    if(abs(num_t[i]-Num_Templete[k][i])<=1)
                        matchnum+=2;
                }
                else if( num_t[i]==Num_Templete[k][i])
                {
                    matchnum+=2;
                }
            }
            if(matchnum>matchnum_max)
            {
                matchnum_max=matchnum;  //保留最大的 匹配
                matchcode= k;  //记录 识别的字符的 索引
                //matchtempnum[j]=matchnum_min
            }
        }
        //识别输出  存放输出结果
        G_PlateChar[char_num]=PlateCode[matchcode]; //保存下该字符
}

void Testcode::OnShibiecar()
{
        // TODO: Add your control notification handler code here
        //---------------------- 车牌识别 -----------------------------//
        CodeRecognize(pImgCharOne,3,0);
        CodeRecognize(pImgCharTwo,1,1);
        CodeRecognize(pImgCharThree,2,2);
        CodeRecognize(pImgCharFour,2,3);
        CodeRecognize(pImgCharFive,0,4);
        CodeRecognize(pImgCharSix,0,5);
        CodeRecognize(pImgCharSeven,0,6);


}

string Testcode::testcode(string filename)
{
        {
            //string s;
            //string s = filename.toStdString();//加入pushbutton 3
           //s="\\""+filename+"\\"";
           const char* ss = filename.c_str();
           src=cvLoadImage(ss, -1 );//filename由上层调用函数得到
           //src=cvLoadImage( "D:/a_project/image/car2.jpg", -1 );//filename由上层调用函数得到
           pImgCanny=cvCreateImage(cvSize(src->width,src->height),IPL_DEPTH_8U,1);     //  2值化后图片大小初始化
           cvCvtColor(src,pImgCanny,CV_RGB2GRAY);  //转化为灰度图  openCV函数 Y = 0.299*R + 0.587*G + 0.114*B
          // cvEqualizeHist(pImgCanny, pImgCanny);//直方图均衡化
           cvSmooth(pImgCanny,pImgCanny,CV_GAUSSIAN,3,0);   //平滑高斯滤波  滤波后的图片保存在 pImgCanny
           Threshold(pImgCanny,pImgCanny);//二值化+边缘检测
           PlateAreaSearch(pImgCanny);    //  车牌定位 并显示
           SegmentPlate();//字符分割
           OnShibiecar();//字符识别

           string outFile= "";
           int i ;
           for(i =0;i<7;i++) //把结果放到 outFile CString 里
           {
              outFile += G_PlateChar[i];
           }
           return outFile;
           //QString qstr2 = QString::fromStdString(outFile);////加入pushbutton 3
        }
}

/*
void MainWindow::on_pushButton_3_clicked()
{
    string s =filename.toStdString();//QString转换为string
    string outFile =string testcode(string s)；
    filename = QString::fromStdString(outFile);////string转换为QString
    ui->lineEdit->setText(filename);
}
*/
