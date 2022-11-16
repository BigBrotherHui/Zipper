#include "zipper.h"
#include <QDirIterator>
#include <QDebug>

Zipper::Zipper(QObject *parent) : QObject(parent)
{

}

Zipper *Zipper::getInstance()
{
    static Zipper zipper;
    return &zipper;
}

void Zipper::ZipSingleFiles(QString qsFilePath,QString rootPath, QuaZip& zip, const char* chPassword)
{
    QFile inFile(qsFilePath);
    if(!inFile.exists())
        return;
    QuaZipFile outFile(&zip);
    if (!inFile.open(QIODevice::ReadOnly))
    {
        return;
    }
    QString fileName=qsFilePath.remove(rootPath);
    if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileName, inFile.fileName()), chPassword))
    {
        return;
    }
    outFile.write(inFile.readAll());
    outFile.close();
    inFile.close();
}

bool Zipper::TraverseFolderZipFiles(QString qsFolder,QString rootPath, QuaZip& zip, const char* chPassword)
{
    QDir dir(qsFolder);
    if (!dir.exists())
    {
        return false; //不存在，直接返回
    }
    dir.setSorting(QDir::DirsFirst); //首先是目录，后是文件
    //获取qsFolder文件夹下所有的子文件夹
    QDirIterator it(qsFolder, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot); //遍历所有目录文件
    //遍历查询出来的子文件夹
    while (it.hasNext()) //有效数据
    {
        QString filename = it.next(); //路径
        QFileInfo fileInfo(filename);
        if (fileInfo.isDir())
        {
            //获取的是文件夹
            QString filepath = fileInfo.filePath();
            //递归遍历
            TraverseFolderZipFiles(filepath,rootPath, zip, chPassword);
        }
        else
        {
            ZipSingleFiles(filename, rootPath, zip, chPassword);
        }
    }
}

bool Zipper::ZipFolder(QString qsFolder, QString qsZipFilePath, const char* chPassword)
{
    QuaZip zip( qsZipFilePath);
    if (!zip.open(QuaZip::mdCreate))
    {
        qDebug()<<"Could not create zip:"+qsZipFilePath;
        return false;
    }
    TraverseFolderZipFiles(qsFolder,qsFolder,zip,chPassword);
    zip.close();
}

void Zipper::ExtractFiles(QuaZip &zip,QString DirPath,const char *password)
{
   zip.goToFirstFile();
   while(zip.hasCurrentFile())
   {
       QuaZipFile zipFile(&zip);
       if(!zipFile.open(QIODevice::ReadOnly,password) || zipFile.getZipError()!=UNZ_OK)
       {
           qDebug()<<"read "<<zip.getCurrentFileName()<<"failed";
           continue;
       }
       QuaZipFileInfo64 fileinfo;
       zip.getCurrentFileInfo(&fileinfo);
       QString outFileName=DirPath+fileinfo.name;
       QFile outFile(outFileName);
       outFile.setPermissions(fileinfo.getPermissions());
       if (fileinfo.isSymbolicLink())
       {
           QString target = QFile::decodeName(zipFile.readAll());
           QFile::link(target, outFileName);
           zipFile.close();
           continue;
       }
       if(!outFile.exists())
       {
           QString path=QFileInfo(outFileName).absolutePath();
           if(!QFileInfo(path).exists())
               QDir().mkpath(path);
       }
       if(!outFile.open(QIODevice::WriteOnly))
       {
           qDebug()<<DirPath+fileinfo.name<<"write failed";
           zipFile.close();
           continue;
       }
       while (!zipFile.atEnd())
       {
           char buf[4096];
           qint64 readLen = zipFile.read(buf, 4096);
           if (readLen <= 0)
           {
               qDebug()<<"read failed";
               break;
           }
           if (outFile.write(buf, readLen) != readLen)
           {
               qDebug()<<"write failed";
               break;
           }
       }
       zipFile.close();
       outFile.close();
       zip.goToNextFile();
   }
}
void Zipper::UnzipFolder(QString qsFolder, QString qsZipFilePath, const char* chPassword)
{
    QFileInfo fileInfo(qsZipFilePath);
    if (!fileInfo.exists())
    {
        qDebug()<<"file not exists";
        return;
    }
    if(fileInfo.exists(qsFolder) && !QDir(qsFolder).isEmpty())
    {
        qDebug()<<"dir is not empty";
        QDir::cleanPath(qsFolder);
    }
    QDir tmpdir;
    if(!tmpdir.mkpath(qsFolder))
    {
        qDebug()<<qsFolder<<"create failed";
        return;
    }
    QuaZip zip(qsZipFilePath);
    if(!zip.open(QuaZip::mdUnzip))
    {
        qDebug()<<"open zip failed";
        return;
    }
    QuaZipFile zipFile(qsZipFilePath);
    if (!zipFile.open(QIODevice::ReadOnly, chPassword))
    {
       qDebug()<<"zip file"<<qsZipFilePath<<" open failed";
       return;
    }
    ExtractFiles(zip,qsFolder,chPassword);
    zipFile.close();
    zip.close();
}

bool Zipper::ZipFile(QString qsFile, QString qsZipFilePath, const char *chPassword)
{
    QuaZip zip(qsZipFilePath);
    if(!zip.open(QuaZip::mdCreate))
    {
        qDebug()<<"zip not create";
        return false;
    }
    QuaZipFile zipFile(&zip);
    if(!zipFile.open(QIODevice::WriteOnly,QuaZipNewInfo(QFileInfo(qsFile).fileName(),qsFile),chPassword,0,8))
    {
        qDebug()<<"zip file not open";
        return false;
    }
    QFile file(qsFile);
    if(!file.exists() || !file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"file not exists or open";
        return false;
    }
    QByteArray ar=file.readAll();
    zipFile.write(ar);
    zipFile.close();
    file.close();
}

void Zipper::UnZipFile(QString qsFile, QString qsZipFilePath, const char *chPassword)
{
    QuaZipFile tZipFile(qsZipFilePath);
    if(!tZipFile.open(QIODevice::ReadOnly,chPassword))
    {
        qDebug()<<"open zipfile failed";
        return;
    }
    QFile file(qsFile);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug()<<"file write failed";
        return;
    }
    file.write(tZipFile.readAll());
    file.close();
    tZipFile.close();
}
