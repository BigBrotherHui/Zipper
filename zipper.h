#ifndef ZIPPER_H
#define ZIPPER_H

#include <QObject>
#include "JlCompress.h"
class Zipper : public QObject
{
public:
    static Zipper *getInstance();
    /*
     *@param1：需要被解压的文件夹（绝对路径）
     *@param2：压缩的路径（绝对路径）
     *@param3：密码
     */
    bool ZipFolder(QString qsFolder, QString qsZipFilePath, const char* chPassword);
    void UnzipFolder(QString qsFolder, QString qsZipFilePath, const char* chPassword);
    bool ZipFile(QString qsFile, QString qsZipFilePath, const char* chPassword);
    void UnZipFile(QString qsFile, QString qsZipFilePath, const char* chPassword);
protected:
    Zipper(QObject *parent=nullptr);
        /*
     *@param1：需要解析的文件夹路径
     *@param1：初始文件夹路径，也就是用户传入的文件夹路径
     *@param1：压缩包创建
     *@param1：密码
     */
    bool TraverseFolderZipFiles(QString qsFolder,QString rootPath, QuaZip& zip, const char* chPassword);  //遍历文件夹压缩文件
    void ZipSingleFiles(QString qsFilePath,QString rootPath, QuaZip& zip, const char* chPassword);
    void ExtractFiles(QuaZip &zip,QString DirPath,const char *password);
};

#endif // ZIPPER_H
