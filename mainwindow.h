#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtCore/QFile>

namespace Ui {
class MainWindow;
}
//Tag Header里存放的是当前Tag的类型、数据区（Tag Data）长度等信息，具体如下：
//名称	长度	介绍
//Tag类型	1 bytes	  8：音频;9：视频;18：脚本；其他：保留
//数据区长度    3 bytes	在数据区的长度
//时间戳       3 bytes	整数，单位是毫秒。对于脚本型的tag总是0
//时间戳扩展	  1 bytes	将时间戳扩展为4bytes，代表高8位。很少用到
//StreamsID	  3 bytes	总是0
typedef struct stTagHeaderInfo{
    quint8  tagType;
    quint8  tagIsVideo;
    quint8  tagIsAudio;
    quint8  tagIsScript;
    quint32 tagDataLenght;
    quint32 tagTimeStamp;
    quint32 tagTimeStampExt;
    quint32 tagStreamId;
} TagHeaderInfo;

//第一个AMF包：
//第1个字节表示AMF包类型，一般总是0x02，表示字符串。
//第2-3个字节为UI16类型值，标识字符串的长度，一般总是0x000A（“onMetaData”长度）。
//后面字节为具体的字符串，一般总为“onMetaData”（6F,6E,4D,65,74,61,44,61,74,61）。
typedef struct stAMFInfoOnMetaData{
    quint8  AMFType;//onMetaData 1字节
    quint32 AMFDataLenght;//2字节
    QString AMFInfoStr;//字符串
} AMFInfoOnMetaData;
//第二个AMF包：
//第1个字节表示AMF包类型，一般总是0x08，表示数组。
//第2-5个字节为UI32类型值，表示数组元素的个数。
//后面即为各数组元素的封装，数组元素为元素名称和值组成的对。
//常见的数组元素如下面所示。：
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
typedef struct stAMFInfoMetaData{
    quint8  AMFType;//onMetaData 1字节
    quint32 elementNum;//第2-5个字节为UI32类型值，表示数组元素的个数。
    double  duration;//时间长度 单位秒
    double  width;//水平像素点数目
    double  height;//水平像素点数目
    double  videodatarate;//视频码率
    double  framerate;//视频帧率
    QString  videocodecid;//字符串 UTF-8
    double  audiosamplerate;//音频采样率
    double  audiosamplesize;//音频采样精度
    double  stereo;//是否为立体声
    double  audiocodecid;//音频编码方式
    double  filesize;//文件大小
    QList<quint32> listAMFElem;
    QMap<QString,double> mapAMFInfo;
} AMFInfoMetaData;
//第一个byte是视频信息，格式如下：
//名称	长度	介绍
//帧类型	4 bits
//    1: keyframe (for AVC, a seekable frame)
//    2: inter frame (for AVC, a non-seekable frame)
//    3: disposable inter frame (H.263 only)
//    4: generated keyframe (reserved for server use only)
//    5: video info/command frame
//编码ID	4 bits
//    1: JPEG (currently unused)
//    2: Sorenson H.263
//    3: Screen video
//    4: On2 VP6
//    5: On2 VP6 with alpha channel
//    6: Screen video version 2
//    7: AVC
typedef struct stVideoTagHeaderInfo{
    quint8  videoTagFrameType;
    quint8  videoTagCodecType;
} VideoTagHeaderInfo;

//AVCVideoPacket格式：分为3种
//AVCVideoPacket同样包括Packet Header和Packet Body两部分：
//即AVCVideoPacket Format：| AVCPacketType(8bit)| CompostionTime(24bit) | Data |
//AVCPacketType为包的类型：
//    如果AVCPacketType=0x00，为AVCSequence Header；
//    如果AVCPacketType=0x01，为AVC NALU；
//    如果AVCPacketType=0x02，为AVC end ofsequence
typedef struct stAVCVideoPacketHeaderInfo{
    quint8  AVCVideoPacketType;
    quint32  CompostionTime;
} AVCVideoPacketHeaderInfo;
//AVCDecorderConfigurationRecord格式
//AVCDecorderConfigurationRecord包括文件的信息。
//具体格式如下：| cfgVersion(8) | avcProfile(8) | profileCompatibility(8)
//           | avcLevel(8) | reserved(6) | lengthSizeMinusOne(2)
//           | reserved(3) | numOfSPS(5) |spsLength(16) | sps(n)
//           | numOfPPS(8) | ppsLength(16) | pps(n) |
typedef struct stAVCDecorderConfigurationRecord{
    quint8  cfgVersion;
    quint8  avcProfile;
    quint8  profileCompatibility;
    quint8  avcLevel;
    quint8  lengthSizeMinusOne;
    quint8  lengthSizeOfNALUnit;//用于表示NAL Unit长度的字节数
    quint8  numOfSPS;
    QList<qint32>  spsLengthList;
    quint8  numOfPPS;
    QList<qint32>  ppsLengthList;
    QList<QByteArray> spsList;
    QList<QByteArray> ppsList;
} AVCDecorderConfigurationRecord;

typedef struct stAudioTagHeaderInfo{
    quint8  audioTagCodecType;//编码器类型
    quint32 audioTagSampleFreq;//采样率 Hz
    quint8  audioTagSampleBit;//采样精度 8比特、16比特
    quint8  audioTagStereo;//是否是双声道
} audioTagHeaderInfo;

typedef struct stAACAdtsHeader{
    quint32  adtsSyncBits;// 12 bit “1111 1111 1111”
    quint8   adtsID;// 1 bit
    quint8   adtsLayer;// 2 bit
    quint8   adtsProtectionAbsent;// 1 bit
    quint8   adtsProfile;// 2 bit 00:Main 01:LC 10:SSR 11:reerved
    quint32  adtsSampleFreqIndex;// 4 bit 采样率 Hz
    quint32  adtsPrivateBit;// 1 bit
    quint32  adtsChannelConfig;// 3 bit
    quint32  adtsOrigCopy;// 1 bit
    quint32  adtsHome;// 1 bit
    quint32  adtsEmphasis;// 2 bit
    //扩展
    quint32  adtsCopyrightIdentBit;// 1 bit
    quint32  adtsCopyrightIdentStart;// 1 bit
    quint32  adtsAacFrameLength;// 13 bit
    quint32  adtsBufferFullness;// 11 bit
    quint32  adtsNumRawBlocksInFrames;// 2 bit
} AACAdtsHeader;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QByteArray writeAdtsHeader(AACAdtsHeader curAACAdtsHeader);
    void initBasicData();

private:
    Ui::MainWindow *ui;
    //cms log文件
    QFile *logFile;
    QFile *fFlvOutput;//输出的flv文件
    QFile *fFlvInput;//输入的flv文件
    QFile *fAVCOutput;//输出的H264文件
    QList<QByteArray> listTag;//存放Tag
    QList<TagHeaderInfo> listTagInfo;//存放Tag
    QList<QString> listVideoCodecName;
    QList<QByteArray> avcNALList;//
    AVCDecorderConfigurationRecord flvAvcDecCfgRec;

    //audio
    AACAdtsHeader curAACAdtsHeader;//
    QList<QString> listAudioCodecName;
    QList<quint32> listAudioSampleFreq;//sampling frequeny [Hz]
    QList<QByteArray> aacNALList;//存放aac的帧数据
};

#endif // MAINWINDOW_H
