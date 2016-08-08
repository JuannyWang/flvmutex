#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化日志、常量等
    initBasicData();

    QTextStream logFileStream(logFile);
    logFileStream<<"start!\n";

    QFile *fAVCOutput;//输出的H264文件
    fAVCOutput = new QFile("out.264");//输出的H264文件
    if(!fAVCOutput->open(QIODevice::WriteOnly))
    {
        logFileStream<<"fAVCOutput out.264 open failed\n";
        exit(-1);
    }
    QDataStream outH264DataStream(fAVCOutput);    // read


    QFile *fAACOutput;//输出的AAC文件
    fAACOutput = new QFile("out.aac");//输出的AAC文件
    if(!fAACOutput->open(QIODevice::WriteOnly))
    {
        logFileStream<<"fAACOutput out.aac open failed\n";
        exit(-1);
    }
    QDataStream outAACDataStream(fAACOutput);    // read

    //720P-american-24-10m-300f.flv
    fFlvInput = new QFile("720P-american-24-10m-1200f.flv");//输入的flv文件
    if(!fFlvInput->open(QIODevice::ReadOnly))
    {
        logFileStream<<"fFlvInput.open failed\n";
        exit(-1);
    }
    QDataStream inDataStream(fFlvInput);    // read the data serialized from the file
    inDataStream.setByteOrder(QDataStream::BigEndian);
    QString sFlvFileHeaderName;
    QString sFlvFileHeaderVersion;
    QString sFlvFileHeaderStreamInfo;
    QString sFlvFileHeaderStreamHaveVideo;
    QString sFlvFileHeaderStreamHaveAudio;
    QString sFlvFileHeaderLength;//整个header的长度
    quint32 ui32FlvHeaderLength;
    quint8 tmpChar;
    //文件类型   3 bytes	“FLV”
    //版本       1 byte	一般为0x01
    //流信息     1 byte	倒数第一位是1表示有视频，倒数第三位是1表示有音频，倒数第二、四位必须为0
    //header长度	4 bytes	整个header的长度，一般为9；大于9表示下面还有扩展信息

    //读取3个字节，“FLV”
    inDataStream>>tmpChar;
    sFlvFileHeaderName+=tmpChar;
    inDataStream>>tmpChar;
    sFlvFileHeaderName+=tmpChar;
    inDataStream>>tmpChar;
    sFlvFileHeaderName+=tmpChar;
    logFileStream<<"sFlvFileHeaderName="<<sFlvFileHeaderName<<"\n";

    inDataStream>>tmpChar;
    sFlvFileHeaderVersion+=QString("%1").arg(tmpChar);
    logFileStream<<"sFlvFileHeaderVersion="<<sFlvFileHeaderVersion<<"\n";


    inDataStream>>tmpChar;
    sFlvFileHeaderStreamInfo+=QString("0x%1").arg(tmpChar,2,16,QLatin1Char('0'));
    logFileStream<<"sFlvFileHeaderStreamInfo="<<sFlvFileHeaderStreamInfo<<"\n";
    quint8 haveVideo,haveAudio;
    haveVideo = (tmpChar&0x01);
    haveAudio = (tmpChar&0x04);
    sFlvFileHeaderStreamHaveVideo =QString("%1").arg(haveVideo);
    sFlvFileHeaderStreamHaveAudio =QString("%1").arg(haveAudio);
    logFileStream<<"sFlvFileHeaderStreamHaveVideo="<<sFlvFileHeaderStreamHaveVideo<<"\n";
    logFileStream<<"sFlvFileHeaderStreamHaveAudio="<<sFlvFileHeaderStreamHaveAudio<<"\n";


    inDataStream>>ui32FlvHeaderLength;
    sFlvFileHeaderLength+=QString("%1").arg(ui32FlvHeaderLength);
    logFileStream<<"sFlvFileHeaderLength="<<sFlvFileHeaderLength<<"\n";

    //如果header长度大于9，说明后面还有扩展字节
    if( ui32FlvHeaderLength >9)
    {
        int i=0;
        QString sHeaderExt;
        for(i=9;i<ui32FlvHeaderLength;i++)
        {
            inDataStream>>tmpChar;
            sHeaderExt+=QString("%1").arg(tmpChar);
        }
        logFileStream<<"sHeaderExt="<<sHeaderExt<<"\n";
    }

    //接下来就是 PreviousTagSize + Tag 的循环
    //直到文件结束
    logFileStream<<"\n\n";
    quint32 ui32TagIdx=0;
    while(!inDataStream.atEnd())
    {
        quint8 tmpUi8;
        quint32 tmpUi32;
        quint32 ui32PrevTagSize;
        QByteArray curTagByteArray;
        TagHeaderInfo curTagHeaderInfo;
        inDataStream >> ui32PrevTagSize;
        logFileStream<<"ui32TagIdx="<<ui32TagIdx<<"\n";
        logFileStream<<"ui32PrevTagSize="<<ui32PrevTagSize<<"\n";
        //Tag Header里存放的是当前Tag的类型、数据区（Tag Data）长度等信息，具体如下：
        //时间戳       3 bytes	整数，单位是毫秒。对于脚本型的tag总是0
        //时间戳扩展	  1 bytes	将时间戳扩展为4bytes，代表高8位。很少用到
        //StreamsID	  3 bytes	总是0

        //Tag类型	1 bytes
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        curTagHeaderInfo.tagType = tmpUi8;
        curTagHeaderInfo.tagIsVideo = (tmpUi8==9);
        curTagHeaderInfo.tagIsAudio = (tmpUi8==8);
        curTagHeaderInfo.tagIsScript = (tmpUi8==18);
        logFileStream<<"tagType="<<curTagHeaderInfo.tagType<<"\n";

        //数据区长度    3 bytes	在数据区的长度
        tmpUi32=0;
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        curTagHeaderInfo.tagDataLenght = tmpUi32;
        logFileStream<<"tagDataLenght="<<curTagHeaderInfo.tagDataLenght<<"\n";

        //时间戳       3 bytes	整数，单位是毫秒。对于脚本型的tag总是0
        tmpUi32=0;
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        curTagHeaderInfo.tagTimeStamp = tmpUi32;
        //时间戳扩展	  1 bytes	将时间戳扩展为4bytes，代表高8位。很少用到
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        curTagHeaderInfo.tagTimeStampExt = tmpUi8;
        tmpUi32 = ((tmpUi8<<24)+tmpUi32);
        curTagHeaderInfo.tagTimeStamp = tmpUi32;
        logFileStream<<"tagTimeStamp="<<curTagHeaderInfo.tagTimeStamp<<"\n";
        //StreamsID	  3 bytes	总是0
        tmpUi32=0;
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        inDataStream >> tmpUi8;
        curTagByteArray.append(tmpUi8);
        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
        curTagHeaderInfo.tagStreamId = tmpUi32;
        logFileStream<<"tagStreamId="<<curTagHeaderInfo.tagStreamId<<"\n";


        int i=0;
        for(i=0;i<curTagHeaderInfo.tagDataLenght;i++)
        {
            inDataStream>>tmpUi8;
            curTagByteArray.append(tmpUi8);
        }
        //        QList<QByteArray> listTagByte;//存放Tag
        //        QList<TagHeaderInfo> listTagInfo;//存放Tag

        //处理Tag Data
        //如果是Script Tag
        if(curTagHeaderInfo.tagIsScript)
        {
            //Tag又通常被称为Metadata Tag，会放一些关于FLV视频和音频的参数信息.
            //如duration、width、height等.
            //通常该类型Tag会跟在File Header后面作为第一个Tag出现，而且只有一个
            // 一般来说，该Tag Data结构包含两个AMF包。
            //AMF（Action Message Format）是Adobe设计的一种通用数据封装格式
            //第一个AMF包封装字符串类型数据，用来装入一个“onMetaData”标志
            //第二个AMF包封装一个数组类型，这个数组中包含了音视频信息项的名称和值
            //            duration      时长
            //            width         视频宽度
            //            height        视频高度
            //            videodatarate	视频码率
            //            framerate     视频帧率
            //            videocodecid	视频编码方式
            //            audiosamplerate	 音频采样率
            //            audiosamplesize	 音频采样精度
            //            stereo             是否为立体声
            //            audiocodecid       音频编码方式
            //            filesize           文件大小
            QDataStream readTagData(curTagByteArray);
            readTagData.setByteOrder(QDataStream::BigEndian);
            readTagData.skipRawData(11);
            quint8 tmpUi8;
            quint32 tmpUi32;
            AMFInfoOnMetaData curAMFInfoOnMetaData;


            //第1个字节表示AMF包类型，一般总是0x02，表示字符串。
            readTagData >> tmpUi8;
            curAMFInfoOnMetaData.AMFType = tmpUi8;

            //第2-3个字节为UI16类型值，标识字符串的长度，一般总是0x000A
            //（“onMetaData”长度）。
            tmpUi32=0;
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            curAMFInfoOnMetaData.AMFDataLenght = tmpUi32;

            //后面字节为具体的字符串，一般总为“onMetaData”
            //（6F,6E,4D,65,74,61,44,61,74,61）
            int i=0;
            QByteArray tmpByteArray;
            for(i=0;i<curAMFInfoOnMetaData.AMFDataLenght;i++)
            {
                readTagData>>tmpUi8;
                tmpByteArray.append(tmpUi8);
            }
            QString tmpStrFromUTF8= QString::fromLatin1(tmpByteArray);
            curAMFInfoOnMetaData.AMFInfoStr = tmpStrFromUTF8;

            logFileStream<<"AMF type:"<<curAMFInfoOnMetaData.AMFType<<"\n";
            logFileStream<<"AMF AMFDataLenght:"<<curAMFInfoOnMetaData.AMFDataLenght<<"\n";
            logFileStream<<"AMF AMFInfoStr:"<<curAMFInfoOnMetaData.AMFInfoStr<<"\n";

            AMFInfoMetaData curAMFInfoMetaData;
            //第1个字节表示AMF包类型，一般总是0x02，表示字符串。
            readTagData >> tmpUi8;
            curAMFInfoMetaData.AMFType = tmpUi8;
            //ecma-array-marker=0x08

            //第2-5个字节为UI32类型值，表示数组元素的个数。
            tmpUi32=0;
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            curAMFInfoMetaData.elementNum=tmpUi32;

            //循环解析 key-value
            //key是UTF8字符串、value是double IEEE-754 8字节，大端字节
            int eleIdx;
            for(eleIdx=0;eleIdx<curAMFInfoMetaData.elementNum;eleIdx++)
            {
                quint32  curElemLenggth;
                QString  curElemString;
                double   curElemValue;
                tmpUi32=0;
                readTagData >> tmpUi8;
                tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                readTagData >> tmpUi8;
                tmpUi32 = ((tmpUi32<<8)+tmpUi8);

                curElemLenggth=tmpUi32;
                curAMFInfoMetaData.listAMFElem.append(curElemLenggth);
                logFileStream<<"\t curElemLenggth:"<<curElemLenggth<<"\n";
                int i=0;
                QByteArray tmpByteArray;
                for(i=0; i<curElemLenggth;i++)
                {
                    readTagData>>tmpUi8;
                    tmpByteArray.append(tmpUi8);
                }
                curElemString = QString::fromLatin1(tmpByteArray);
                //读取1个字节 0x00 : UTF-8-empty object-end-marker


                readTagData >> tmpUi8;
                //表示接下来的是字符串
                if(tmpUi8 == 2)
                {
                    quint32 tmplength;

                    tmpUi32=0;
                    readTagData >> tmpUi8;
                    tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                    readTagData >> tmpUi8;
                    tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                    logFileStream<<"\t length:"<<tmpUi32<<"\n";
                    tmplength = tmpUi32;
                    int i=0;
                    QByteArray tmpByteArray;
                    for(i=0; i<tmplength;i++)
                    {
                        readTagData>>tmpUi8;
                        tmpByteArray.append(tmpUi8);
                    }
                    QString tmpEncoderString = QString::fromLatin1(tmpByteArray);

                    logFileStream<<"\t tmpEncoderString:"<<tmpEncoderString<<"\ns";
                }
                else if(tmpUi8==0)//接下来是8字节 double
                {
                    //读取8个字节 value是double IEEE-754 8字节，大端字节
                    readTagData >> curElemValue;
                }
                else if(tmpUi8==1)//接下来是1字节 boolean
                {
                    //接下来是1字节 boolean
                    readTagData >> tmpUi8;
                    logFileStream<<"\t boolean:"<<tmpUi8<<"\n";
                }
                else
                {
                    logFileStream<<"Unknow type. Support string\double type!\n";
                }
                logFileStream<<"AMF elem["<<eleIdx<<"] "<<curElemString<<"="<<curElemValue<<"\n";
            }

        }
        //如果是Video Tag
        else if(curTagHeaderInfo.tagIsVideo)
        {

            QDataStream readTagData(curTagByteArray);
            readTagData.setByteOrder(QDataStream::BigEndian);
            readTagData.skipRawData(11);//跳过Tag header的11个字节
            VideoTagHeaderInfo curVideoTagHeaderInfo;

            quint8 tmpUi8;
            quint32 tmpUi32;


            //第1个字节表示 video 的帧类型、编码格式
            readTagData >> tmpUi8;
            curVideoTagHeaderInfo.videoTagFrameType = (tmpUi8>>4);
            curVideoTagHeaderInfo.videoTagCodecType = (tmpUi8&0x0F);

            logFileStream<<"VideoCodecName:"<<listVideoCodecName.at(curVideoTagHeaderInfo.videoTagCodecType);
            logFileStream<<"\n";
            //如果是 AVC video Packet
            if(curVideoTagHeaderInfo.videoTagCodecType == 7)
            {
                //AVCVideoPacket格式：分为3种
                //AVCVideoPacket同样包括Packet Header和Packet Body两部分：
                //即AVCVideoPacket Format：| AVCPacketType(8bit)| CompostionTime(24bit) | Data |
                //AVCPacketType为包的类型：
                //    如果AVCPacketType=0x00，为AVCSequence Header；
                //    如果AVCPacketType=0x01，为AVC NALU；
                //    如果AVCPacketType=0x02，为AVC end ofsequence
                AVCVideoPacketHeaderInfo curAVCVideoPacketHeaderInfo;
                readTagData >> tmpUi8;
                curAVCVideoPacketHeaderInfo.AVCVideoPacketType = tmpUi8;

                tmpUi32=0;
                readTagData >> tmpUi8;
                tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                readTagData >> tmpUi8;
                tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                readTagData >> tmpUi8;
                tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                curAVCVideoPacketHeaderInfo.CompostionTime = tmpUi32;

                //如果AVCPacketType=0x00，为AVCSequence Header；
                if(curAVCVideoPacketHeaderInfo.AVCVideoPacketType == 0)
                {
                    AVCDecorderConfigurationRecord curAVCDecCfgRecord;
                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.cfgVersion = tmpUi8;
                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.avcProfile = tmpUi8;
                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.profileCompatibility = tmpUi8;
                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.avcLevel = tmpUi8;
                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.lengthSizeMinusOne = (tmpUi8&0x03);
                    curAVCDecCfgRecord.lengthSizeOfNALUnit = curAVCDecCfgRecord.lengthSizeMinusOne+1;
                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.numOfSPS = (tmpUi8&0x1F);
                    int tmpIdx=0;
                    for(tmpIdx=0; tmpIdx<curAVCDecCfgRecord.numOfSPS;tmpIdx++)
                    {
                        quint32 curSPSLenggth;
                        tmpUi32=0;
                        readTagData >> tmpUi8;
                        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                        readTagData >> tmpUi8;
                        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                        curSPSLenggth = tmpUi32;//字节数
                        int i=0;
                        QByteArray tmpByteArray;
                        logFileStream<<"SPS["<<tmpIdx<<"]: 0x ";
                        //写入NAL的起始字节 0x 00 00 01
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x01;
                        for(i=0; i<curSPSLenggth;i++)
                        {
                            readTagData>>tmpUi8;
                            tmpByteArray.append(tmpUi8);
                            QString tmpStr=QString("%1 ").arg(tmpUi8,2,16,QLatin1Char('0'));
                            tmpStr=tmpStr.toUpper();
                            logFileStream<<tmpStr;
                            outH264DataStream<<(quint8)tmpUi8;
                        }
                        logFileStream<<"\n";
                        curAVCDecCfgRecord.spsList.append(tmpByteArray);
                    }

                    readTagData >> tmpUi8;
                    curAVCDecCfgRecord.numOfPPS = (tmpUi8&0x1F);
                    for(tmpIdx=0; tmpIdx<curAVCDecCfgRecord.numOfSPS;tmpIdx++)
                    {
                        quint32 curPPSLenggth;
                        tmpUi32=0;
                        readTagData >> tmpUi8;
                        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                        readTagData >> tmpUi8;
                        tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                        curPPSLenggth = tmpUi32;//字节数
                        int i=0;
                        QByteArray tmpByteArray;
                        logFileStream<<"PPS["<<tmpIdx<<"]: 0x ";
                        //写入NAL的起始字节 0x 00 00 01
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x01;
                        for(i=0; i<curPPSLenggth;i++)
                        {
                            readTagData>>tmpUi8;
                            tmpByteArray.append(tmpUi8);
                            QString tmpStr=QString("%1 ").arg(tmpUi8,2,16,QLatin1Char('0'));
                            tmpStr=tmpStr.toUpper();
                            logFileStream<<tmpStr;
                            outH264DataStream<<(quint8)tmpUi8;
                        }
                        logFileStream<<"\n";
                        curAVCDecCfgRecord.ppsList.append(tmpByteArray);
                    }

                    //保存到全局
                    flvAvcDecCfgRec = curAVCDecCfgRecord;
                }
                //如果AVCPacketType=0x01，为AVC NALU；
                else if(curAVCVideoPacketHeaderInfo.AVCVideoPacketType == 1)
                {
                    // 当前Tag中剩余的字节数
                    quint32 byteLeftForNAL;
                    byteLeftForNAL =curTagByteArray.size()-11-1-4;
                    quint32 NALIdx=0;
                    while( byteLeftForNAL >0 && (!readTagData.atEnd()))
                    {
                        quint32 curNALByteNum;
                        int i=0;
                        tmpUi32=0;
                        for(i=0; i<flvAvcDecCfgRec.lengthSizeOfNALUnit ;i++)
                        {
                            readTagData >> tmpUi8;
                            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
                        }
                        curNALByteNum =  tmpUi32;

                        //写入NAL的起始字节 0x 00 00 01
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x00;
                        outH264DataStream<<(quint8)0x01;
                        QByteArray tmpByteArray;
                        for(i=0; i<curNALByteNum;i++)
                        {
                            readTagData>>tmpUi8;
                            tmpByteArray.append(tmpUi8);
                            outH264DataStream<<tmpUi8;
                        }
                        //avcNALList.append(tmpByteArray);
                        byteLeftForNAL=byteLeftForNAL-4-curNALByteNum;
                        logFileStream<<"NAL "<<NALIdx<<": byteNum="<<curNALByteNum<<"\n";
                        NALIdx++;
                    }
                }
                //如果AVCPacketType=0x02，为AVC end ofsequence
                else if(curAVCVideoPacketHeaderInfo.AVCVideoPacketType == 2)
                {

                }
            }
            //第2-3个字节为UI16类型值，标识字符串的长度，一般总是0x000A
            //（“onMetaData”长度）。
            tmpUi32=0;
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);
            readTagData >> tmpUi8;
            tmpUi32 = ((tmpUi32<<8)+tmpUi8);

        }
        //如果是Audio Tag
        else if(curTagHeaderInfo.tagIsAudio)
        {
            QDataStream readTagData(curTagByteArray);
            readTagData.setByteOrder(QDataStream::BigEndian);
            readTagData.skipRawData(11);//跳过Tag header的11个字节
            audioTagHeaderInfo curAudioTagHeaderInfo;

            quint8 tmpUi8;
            quint32 tmpUi32;


            //第1个字节表示 video 的帧类型、编码格式
            readTagData >> tmpUi8;
            curAudioTagHeaderInfo.audioTagCodecType = (tmpUi8>>4);
            curAudioTagHeaderInfo.audioTagSampleFreq = listAudioSampleFreq[((tmpUi8>>2)&0x3)];
            if( ((tmpUi8>>1)&0x1) )
                curAudioTagHeaderInfo.audioTagSampleBit = 16;//
            else
                curAudioTagHeaderInfo.audioTagSampleBit = 8;//

            curAudioTagHeaderInfo.audioTagStereo = (tmpUi8&0x1);

            logFileStream<<"audioTagCodecType: "<<listAudioCodecName[curAudioTagHeaderInfo.audioTagCodecType]<<"\n";
            logFileStream<<"audioTagSampleFreq: "<<curAudioTagHeaderInfo.audioTagSampleFreq<<"\n";
            logFileStream<<"audioTagSampleBit: "<<curAudioTagHeaderInfo.audioTagSampleBit<<"\n";
            logFileStream<<"audioTagStereo: "<<curAudioTagHeaderInfo.audioTagStereo<<"\n";
            //如果是AAC 音频，则需要读取1个字节，来判断TAG的数据类型
            if(curAudioTagHeaderInfo.audioTagCodecType == 10 )
            {
                //0x00:0 = AAC sequence header
                //0x01:1 = AAC raw
                //第1个字节表示 video 的帧类型、编码格式
                readTagData >> tmpUi8;

                //AudioSpecificConfig
                if(tmpUi8 == 0)
                {
                    // 当前Tag中剩余的字节数
                    quint32 byteLeftForNAL;
                    byteLeftForNAL =curTagByteArray.size()-11-1-1;
                    logFileStream<<"AAC byteNum="<<byteLeftForNAL<<"\n";

                    //首先读取5比特：audioObjectType，如果数值为31，则再读取6比特的扩展。
                    //audioObjectType读取00010=2：AAC LC ISO/IEC 14496-3 subpart 4
                    readTagData >> tmpUi8;
                    //00001: Main ==> 00:Main
                    //00010: LC   ==> 00:LC
                    //00011: SSR  ==> 00:SSR
                    quint32 ui32AACProfile=tmpUi8>>3;
                    quint32 ui32AACProfileExt=0;
                    if(ui32AACProfile == 1)
                        curAACAdtsHeader.adtsProfile = 0;// 2 bit 00:Main 01:LC 10:SSR 11:reerved
                    else if(ui32AACProfile == 2)
                        curAACAdtsHeader.adtsProfile = 1;// 2 bit 00:Main 01:LC 10:SSR 11:reerved
                    else if(ui32AACProfile == 3)
                        curAACAdtsHeader.adtsProfile = 2;// 2 bit 00:Main 01:LC 10:SSR 11:reerved
                    else
                        logFileStream<<"ADTS not support Profile:"<<ui32AACProfile<<" "<<"\n";
                    logFileStream<<"AudioSpecificConfig Profile:"<<ui32AACProfile<<" "<<"\n";

                    if(ui32AACProfile == 31)// 继续读取6比特
                    {
                        ui32AACProfileExt=tmpUi8&0x7;
                        readTagData >> tmpUi8;
                        ui32AACProfileExt= (ui32AACProfileExt<<3) + (tmpUi8>>5);
                    }
                    logFileStream<<"AudioSpecificConfig Profile Ext:"<<ui32AACProfileExt<<" "<<"\n";

                    //读取4比特：samplingFrequencyIndex，如果数值为15，则再读取24比特的自定义采样频率。
                    //samplingFrequencyIndex读取0100=4
                    quint32 ui32SampleFreqIndex = tmpUi8&0x7;//读取剩余3比特
                    readTagData >> tmpUi8;
                    ui32SampleFreqIndex= (ui32SampleFreqIndex<<1) + (tmpUi8>>7);
                    curAACAdtsHeader.adtsSampleFreqIndex = ui32SampleFreqIndex;// 4 bit 采样率 Hz
                    logFileStream<<"AudioSpecificConfig ui32SampleFreqIndex:"<<ui32SampleFreqIndex<<" "<<"\n";

                    //然后读取4比特：channelConfiguration;
                    //channelConfiguration读取0010=2
                    quint32 ui32ChannelConfi = ((tmpUi8>>3)&0xF);
                    curAACAdtsHeader.adtsChannelConfig = ui32ChannelConfi;//
                    logFileStream<<"AudioSpecificConfig ui32ChannelConfi:"<<ui32ChannelConfi<<" "<<"\n";

                    int i=0;

                    //写入ADTS 的头部7字节
                    curAACAdtsHeader.adtsSyncBits=0xFFF;
                    curAACAdtsHeader.adtsID=0;
                    curAACAdtsHeader.adtsLayer=0;
                    curAACAdtsHeader.adtsProtectionAbsent=1;
                    //curAACAdtsHeader.adtsProfile;// 2 bit 00:Main 01:LC 10:SSR 11:reerved
                    //curAACAdtsHeader.adtsSampleFreqIndex;// 4 bit 采样率 Hz
                    curAACAdtsHeader.adtsPrivateBit = 0;// 1 bit
                    //curAACAdtsHeader.adtsChannelConfig;// 3 bit
                    curAACAdtsHeader.adtsOrigCopy =0;// 1 bit
                    curAACAdtsHeader.adtsHome = 0;// 1 bit

                    //扩展
                    curAACAdtsHeader.adtsCopyrightIdentBit = 0;// 1 bit
                    curAACAdtsHeader.adtsCopyrightIdentStart = 0;// 1 bit
                    curAACAdtsHeader.adtsAacFrameLength = 0;// 13 bit
                    curAACAdtsHeader.adtsBufferFullness = 0x7FF;// 11 bit
                    curAACAdtsHeader.adtsNumRawBlocksInFrames = 0;// 2 bit

                    QByteArray tmpByteArray;
                    for(i=0; i<byteLeftForNAL;i++)
                    {
                        readTagData>>tmpUi8;
                        tmpByteArray.append(tmpUi8);
                        QString tmpStr=QString("%1 ").arg(tmpUi8,2,16,QLatin1Char('0'));
                        tmpStr=tmpStr.toUpper();
                        logFileStream<<tmpStr;
                    }
                    logFileStream<<"\n";
                }
                else if(tmpUi8 == 1)
                {
                    // 当前Tag中剩余的字节数
                    quint32 byteLeftForNAL;
                    byteLeftForNAL =curTagByteArray.size()-11-1-1;
                    logFileStream<<"AAC byteNum="<<byteLeftForNAL<<"\n";

                    int i=0;

                    //写入ADTS 的头部7字节
                    curAACAdtsHeader.adtsAacFrameLength = 7+byteLeftForNAL;// 13 bit
                    curAACAdtsHeader.adtsBufferFullness = 0x7FF;// 11 bit
                    curAACAdtsHeader.adtsNumRawBlocksInFrames = 0;// 2 bit

                    QByteArray adtsHeaderByes =writeAdtsHeader(curAACAdtsHeader);
                    for(i=0; i<adtsHeaderByes.size();i++)
                    {
                        tmpUi8 = adtsHeaderByes.at(i);
                        outAACDataStream<<(quint8)tmpUi8;

                        QString tmpStr=QString("%1 ").arg(tmpUi8,2,16,QLatin1Char('0'));
                        tmpStr=tmpStr.toUpper();
                        logFileStream<<tmpStr;
                    }

                    QByteArray tmpByteArray;
                    for(i=0; i<byteLeftForNAL;i++)
                    {
                        readTagData>>tmpUi8;
                        tmpByteArray.append(tmpUi8);
                        outAACDataStream<<tmpUi8;
                        QString tmpStr=QString("%1 ").arg(tmpUi8,2,16,QLatin1Char('0'));
                        tmpStr=tmpStr.toUpper();
                        logFileStream<<tmpStr;
                    }
                    logFileStream<<"\n";
                    //aacNALList.append(tmpByteArray);

                }
                else
                {
                    logFileStream<<"Unknow AAC Tag type. Support 0/1 type!\n";
                }

            }

        }
        else
        {
            logFileStream<<"Unknow Tag type. Support Video\Audio\Script type!\n";
        }
        //VideoTagHeaderInfo


        listTag.append(curTagByteArray);
        listTagInfo.append(curTagHeaderInfo);
        ui32TagIdx++;
        logFileStream<<"\n\n";

    }
    //QList<QByteArray> ;;//存放Tag
}

MainWindow::~MainWindow()
{
    delete ui;
    fAVCOutput->close();
}

void MainWindow::initBasicData()
{
    //初始化日志文件
    logFile = new QFile("log.txt");
    if(!logFile->open(QIODevice::WriteOnly)){
        qDebug()<<"Cannot open file: "<<"log.txt";
    }
    else{
        qDebug()<<"open file: "<<"log.txt";
    }

    //    1: JPEG (currently unused)
    //    2: Sorenson H.263
    //    3: Screen video
    //    4: On2 VP6
    //    5: On2 VP6 with alpha channel
    //    6: Screen video version 2
    //    7: AVC
    listVideoCodecName.append("NULL");
    listVideoCodecName.append("JPEG");
    listVideoCodecName.append("Sorenson H.263");
    listVideoCodecName.append("Screen video");
    listVideoCodecName.append("On2 VP6");
    listVideoCodecName.append("On2 VP6 with alpha channel");
    listVideoCodecName.append("Screen video version 2");
    listVideoCodecName.append("AVC");

    //
    //0	 Linear PCM，platform endian
    //1	 ADPCM
    //2	 MP3
    //3	 Linear PCM，little endian
    //4	 Nellymoser 16-kHz mono
    //5	 Nellymoser 8-kHz mono
    //6	 Nellymoser
    //7	 G.711 A-law logarithmic PCM
    //8	 G.711 mu-law logarithmic PCM
    //9	 reserved
    //10	 AAC
    //14	 MP3 8-Khz
    //15	 Device-specific sound
    listAudioCodecName.append("Linear PCM，platform endian");   //0
    listAudioCodecName.append("ADPCM");                         //1
    listAudioCodecName.append("MP3");                           //2
    listAudioCodecName.append("Linear PCM，little endian");     //3
    listAudioCodecName.append("Nellymoser 16-kHz mono");        //4
    listAudioCodecName.append("Nellymoser 8-kHz mono");         //5
    listAudioCodecName.append("Nellymoser");                    //6
    listAudioCodecName.append("G.711 A-law logarithmic PCM");    //7
    listAudioCodecName.append("G.711 mu-law logarithmic PCM");    //8
    listAudioCodecName.append("reserved");                          //9
    listAudioCodecName.append("AAC");                               //10
    listAudioCodecName.append("null");
    listAudioCodecName.append("null");
    listAudioCodecName.append("null");
    listAudioCodecName.append("null");
    listAudioCodecName.append("MP3 8-Khz");                         //14
    listAudioCodecName.append("Device-specific sound");             //15

    //sampling frequeny [Hz]
    //0	 5.5kHz
    //1	 11KHz
    //2	 22 kHz
    //3	 44 kHz
    listAudioSampleFreq.append(5500);
    listAudioSampleFreq.append(11000);
    listAudioSampleFreq.append(22000);
    listAudioSampleFreq.append(44000);
}

QByteArray MainWindow::writeAdtsHeader(AACAdtsHeader curAACAdtsHeader)
{
    QByteArray outByte;
    quint8 tmpui8;
    quint32 tmpui32;
    tmpui32=0;
    tmpui32 = (tmpui32<<12)+ 0xFFF;//12bit 同步比特
    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsID;//1 bit ID
    tmpui32 = (tmpui32<<2)+ curAACAdtsHeader.adtsLayer;//2 bit layer
    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsProtectionAbsent;//1 bit Protection absent
    //16比特
    tmpui32 = (tmpui32<<2)+ curAACAdtsHeader.adtsProfile;//2 bit Profile
    tmpui32 = (tmpui32<<4)+ curAACAdtsHeader.adtsSampleFreqIndex;//4 bit sample frequency index
    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsPrivateBit;//1 bit private bit
    tmpui32 = (tmpui32<<3)+ curAACAdtsHeader.adtsChannelConfig;//3 bit channel configuration
    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsOrigCopy;//1 bit original copy
    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsHome;//1 bit home

    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsCopyrightIdentBit;     //1 bit copyright identification bit
    tmpui32 = (tmpui32<<1)+ curAACAdtsHeader.adtsCopyrightIdentStart;   //1 bit copyright identification start
    //30比特，写出前24比特，3个字节
    tmpui32 = (tmpui32<<2);
    outByte.append((tmpui32>>24)&0xFF);
    outByte.append((tmpui32>>16)&0xFF);
    outByte.append((tmpui32>>8)&0xFF);

    //还有6比特，存放在低6位
    tmpui32 = (tmpui32>>2);
    tmpui32 = (tmpui32&0xFF);

    tmpui32 = (tmpui32<<13)+ curAACAdtsHeader.adtsAacFrameLength;//13bit aac frame length
    tmpui32 = (tmpui32<<11)+ curAACAdtsHeader.adtsBufferFullness;//11bit aac buffer fullness
    tmpui32 = (tmpui32<<2)+ curAACAdtsHeader.adtsID;//2 bit number of raw data blocks in frames

    //32比特，写32比特，4个字节
    outByte.append((tmpui32>>24)&0xFF);
    outByte.append((tmpui32>>16)&0xFF);
    outByte.append((tmpui32>>8)&0xFF);
    outByte.append((tmpui32)&0xFF);
    //总共7字节
    if(outByte.size() !=7 )
    {
        exit(-2);
    }
    return outByte;
}
