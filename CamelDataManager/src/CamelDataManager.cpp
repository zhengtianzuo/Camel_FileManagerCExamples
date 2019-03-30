﻿/*!
 *@file CamelDataManager.h
 *@brief CamelDataManager
 *@version 1.0
 *@section LICENSE Copyright (C) 2003-2103 CamelSoft Corporation
 *@author zhengtianzuo
*/
#include "CamelDataManager.h"
#include <QFileInfo>
#include <QDateTime>

CamelDataManager::CamelDataManager()
{
    Sub_FMInterface = new Camel_FileManagerCInterface();
    Sub_FMInt = Sub_FMInterface->Cls_funInitialize();
    Sub_FMInt->Cls_funManagerInitialize();
    clsFileManager_CmdMapInit();
    m_tableData = new DataTableViewModel();
    //    for (int i = 0; i < 100; i++)
    //    {
    //        m_tableData->add(QStringLiteral("数据")+QString::number(i), QStringLiteral("文件"), QStringLiteral("1.234M"));
    //    }
    m_listData = new DataListViewModel();
    //    for (int i = 0; i < 100; i++)
    //    {
    //        m_listData->add(QStringLiteral("名称")+QString::number(i), QStringLiteral("路径"));
    //    }
}

CamelDataManager::~CamelDataManager()
{
    delete m_tableData;
    delete m_listData;
    Sub_FMInterface->Cls_funTerminate(Sub_FMInt);
    delete Sub_FMInterface;
}

DataTableViewModel *CamelDataManager::getTableData()
{
    return (m_tableData);
}

DataListViewModel *CamelDataManager::getListData()
{
    return (m_listData);
}

void CamelDataManager::Sub_funManagerBytes(
        int intFunction, void *pContext, char *&pUserData,
        int &intUserSize, void *pReturnData, int intReturnSize)
{
    Q_UNUSED(pContext);
    Q_UNUSED(intUserSize);
    switch (intFunction)
    {
    case clsFileManager_intDataType_String:
    case clsFileManager_intDataType_File:
    {
        pUserData = new char[intReturnSize];
        memcpy(pUserData, pReturnData, intReturnSize);
    }
        break;
    default:
        break;
    }
}

void CamelDataManager::Sub_funManagerData(
        int intFunction, void *pContext, void *pUserData,
        int intUserSize, void *pReturnData, int intReturnSize)
{
    Q_UNUSED(pContext);
    Q_UNUSED(intUserSize);
    Q_UNUSED(intReturnSize);
    Cls_stuFMCmdMap[intFunction].lpFunc(pUserData, pReturnData);
}

int CamelDataManager::cls_funManagerDB_CreateDataBase(QString strName, QString strPass, QString strPath)
{
    string sDBName = strName.toLocal8Bit().toStdString();
    string sDBPass = strPass.toStdString();
    string sAppPath = strPath.toLocal8Bit().toStdString();
    string sDBFileName = sAppPath + sDBName + DBSuffix.toStdString();

    Cls_stuDBVerify dBVerify(sDBFileName.c_str(), sDBPass.c_str());
    int intError = Sub_FMInt->Cls_funManagerDB_CreateDataBase(&dBVerify, sDBName.c_str(), true);
    if (intError == clsFileManager_intErrorCode_Success)
    {
        m_listData->add(strName, strPath);
        m_strCurDBName = sDBName;
        m_strCurDBPass = sDBPass;
        m_strCurDBPath = sDBFileName;
    }
    return(intError);
}

int CamelDataManager::cls_funManagerDB_OpenDataBase(QString strName, QString strPass, QString strPath)
{
    if (strName.endsWith(DBSuffix)) strName.replace(DBSuffix, "");
    if (strPath.endsWith(DBSuffix)) strPath.replace(DBSuffix, "");
    string sDBName = strName.toLocal8Bit().toStdString();
    string sDBPass = strPass.toStdString();
    string sAppPath = strPath.toLocal8Bit().toStdString();
    string sDBFileName = sAppPath + sDBName + DBSuffix.toStdString();

    Cls_stuDBVerify dBVerify(sDBFileName.c_str(), sDBPass.c_str());
    int intError = Sub_FMInt->Cls_funManagerDB_OpenDataBase(&dBVerify);
    if (intError == clsFileManager_intErrorCode_Success)
    {
        int nFileSize = 0;
        if (strName.length() == 0)
        {
            QFileInfo fileInfo;
            fileInfo.setFile(sDBFileName.c_str());
            strName = fileInfo.fileName();
            strName = strName.replace(DBSuffix, "");
            nFileSize = static_cast<int>(fileInfo.size());
        }
        m_listData->add(strName, strPath);
        m_strCurDBName = sDBName;
        m_strCurDBPass = sDBPass;
        m_strCurDBPath = sDBFileName;

        int nFileNum = 0;
        Sub_FMInt->Cls_funManagerDB_GetTotalNumber(&dBVerify, nFileNum);

        char chrDBName[Cls_intFNameSize + 1];
        Sub_FMInt->Cls_funManagerDB_GetName(&dBVerify, chrDBName);

        emit sOpenDataBase(QString(chrDBName).trimmed(), sDBFileName.c_str(), QString::number(nFileNum), size2String(nFileSize));
    }
    return(intError);
}

int CamelDataManager::cls_funManagerDB_SetName(QString strDBName)
{
    string sDBName = strDBName.toLocal8Bit().toStdString();
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    int intError = Sub_FMInt->Cls_funManagerDB_SetName(&dBVerify, sDBName.c_str());
    return(intError);
}

int CamelDataManager::cls_funManagerDB_ChangePassword(QString strNewPass)
{
    string sDBPass = strNewPass.toStdString();
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    int intError = Sub_FMInt->Cls_funManagerDB_ChangePassword(&dBVerify, m_strCurDBPass.c_str(), sDBPass.c_str());
    return(intError);
}

int CamelDataManager::cls_funManagerDB_CheckSpace()
{
    return(0);
}

int CamelDataManager::type2RealType(int nType)
{
    nType++;
    if (nType > 8) nType++;
    return(nType);
}

QString CamelDataManager::type2String(int nType)
{
    switch (nType) {
    case 1:
        return (QString("Short"));
    case 2:
        return (QString("Integer"));
    case 3:
        return (QString("Float"));
    case 4:
        return (QString("Double"));
    case 5:
        return (QString("Currency"));
    case 6:
        return (QString("Date"));
    case 7:
        return (QString("String"));
    case 8:
        return (QString("File"));
    case 10:
        return (QString("ByteArray"));
    case 11:
        return (QString("ShortArray"));
    case 12:
        return (QString("IntegerArray"));
    case 13:
        return (QString("FloatArray"));
    case 14:
        return (QString("DoubleArray"));
    case 15:
        return (QString("CurrencyArray"));
    case 16:
        return (QString("DateArray"));
    case 17:
        return (QString("StringArray"));
    default:
        return ("");
    }
}

QString CamelDataManager::size2String(int nSize)
{
    if (nSize < 1024){
        return (QString::number(nSize)+QString(" B"));
    }else if (nSize < 1024*1024){
        return (QString::number(nSize/1024)+QString(" KB"));
    }else if (nSize < 1024*1024*1024){
        return (QString::number(nSize/1024/1024)+QString(" MB"));
    }else{
        return (QString::number(nSize/1024/1024/1024)+QString(" GB"));
    }
}

int CamelDataManager::cls_funManagerData_Combine(int nDataType, QString strName, QString strValue)
{
    nDataType = type2RealType(nDataType);
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    string sName = strName.toStdString();
    Cls_stuDataType sDataType(nDataType, -1, sName.c_str());
    int intError = 0;
    Cls_lpstuUserData sUserData = nullptr;
    string strData = "";
    switch (nDataType) {
    case 1:
    {
        short sData = strValue.toShort();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 2:
    {
        int sData = strValue.toInt();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 3:
    {
        float sData = strValue.toFloat();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 4:
    {
        double sData = strValue.toDouble();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 5:
    {
        double sData = strValue.toDouble();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 6:
    {
        double sData = strValue.toDouble();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 7:
    {
        strData = strValue.toLocal8Bit().data();
        sUserData = new Cls_stuUserData(&strData, 0);
    }
        break;
    case 8:
    {
    }
        break;
    case 10:
    {
    }
        break;
    case 11:
    {
    }
        break;
    case 12:
    {
    }
        break;
    case 13:
    {
    }
        break;
    case 14:
    {
    }
        break;
    case 15:
    {
    }
        break;
    case 16:
    {
    }
        break;
    case 17:
    {
    }
        break;
    default:
        return (-1);
    }
    intError = Sub_FMInt->Cls_funManagerData_Combine(&dBVerify, &sDataType, nullptr, sUserData, false, -1);
    if (intError == clsFileManager_intErrorCode_Success){
        int intSize = 0;
        intError = Sub_FMInt->Cls_funManagerData_GetSize(&dBVerify, clsFileManager_intSizeType_DataSize, &sDataType, intSize);
        if (intError == clsFileManager_intErrorCode_Success){
            m_tableData->add(strName, type2String(nDataType), size2String(intSize));
        }
    }
    if (sUserData != nullptr){delete sUserData;}
    return(intError);
}

int CamelDataManager::cls_funManagerData_GetAllList()
{
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    Cls_stuFunction funData(&Sub_funManagerData, this);
    vector<string> aryDataNameList;
    Cls_stuUserData userData(&aryDataNameList, -1);
    int intError = Sub_FMInt->Cls_funManagerData_GetNameList(&dBVerify, clsFileManager_intNameType_Name, &funData, &userData);
    if (intError != clsFileManager_intErrorCode_Success) return(intError);

    vector<int> aryDataTypeList;
    Cls_stuUserData userData1(&aryDataTypeList, -1);
    intError = Sub_FMInt->Cls_funManagerData_GetTypeList(&dBVerify, &funData, &userData1);
    if (intError != clsFileManager_intErrorCode_Success) return(intError);
    if (aryDataTypeList.size() != aryDataNameList.size()) return (-1);

    vector<int> aryDataSizeList;
    Cls_stuUserData userData2(&aryDataSizeList, -1);
    intError = Sub_FMInt->Cls_funManagerData_GetSizeList(&dBVerify, clsFileManager_intSizeType_DataSize, &funData, &userData2);
    if (intError != clsFileManager_intErrorCode_Success) return(intError);
    if (aryDataSizeList.size() != aryDataNameList.size()) return (-1);

    for (unsigned int i = 0; i < aryDataNameList.size(); i++){
        m_tableData->add(QString(aryDataNameList.at(i).c_str()), type2String(aryDataTypeList.at(i)), size2String(aryDataSizeList.at(i)));
    }

    return(intError);
}

QString CamelDataManager::cls_funManagerData_GetName(int nRow)
{
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    Cls_stuDataType sDType(-1, nRow+2, nullptr);
    char chrDataName[Cls_intFNameSize + 1];
    int nError = Sub_FMInt->Cls_funManagerData_GetName(&dBVerify, &sDType, clsFileManager_intNameType_Name, chrDataName);
    if (nError != clsFileManager_intErrorCode_Success) return("");
    return(QString(chrDataName));
}

int CamelDataManager::cls_funManagerData_GetType(int nRow)
{
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    Cls_stuDataType sDType(-1, nRow+2, nullptr);
    int nType = 0;
    int nError = Sub_FMInt->Cls_funManagerData_GetType(&dBVerify, &sDType, nType);
    if (nError != clsFileManager_intErrorCode_Success) return(-1);
    return(nType);
}

QString CamelDataManager::cls_funManagerData_GetData(int nRow)
{
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    Cls_stuDataType sDType(-1, nRow+2, nullptr);
    int nType = 0;
    int nError = Sub_FMInt->Cls_funManagerData_GetType(&dBVerify, &sDType, nType);
    if (nError != clsFileManager_intErrorCode_Success) return(QString(""));
    Cls_stuFunction fun(nullptr, nullptr);
    Cls_stuFunction funData(&Sub_funManagerData, this);
    Cls_stuDataType sDataType(nType, nRow+2, nullptr);
    int intSize = 0;
    int intError = 0;
    switch (nType) {
    case 1:
    {
        short srtShort = 0;
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(srtShort), intSize);
        intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &fun, &getUserData);
        if (intError != clsFileManager_intErrorCode_Success) return(QString(""));
        return (QString::number(srtShort));
    }
    case 2:
    {
        int intInt = 0;
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(intInt), intSize);
        intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &fun, &getUserData);
        if (intError != clsFileManager_intErrorCode_Success) return(QString(""));
        return (QString::number(intInt));
    }
    case 3:
    {
        float fltFloat = 0.0;
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(fltFloat), intSize);
        intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &fun, &getUserData);
        if (intError != clsFileManager_intErrorCode_Success) return(QString(""));
        return (QString::number(fltFloat));
    }
        break;
    case 4:
    {
        double dblDouble = 0.0;
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(dblDouble), intSize);
        intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &fun, &getUserData);
        if (intError != clsFileManager_intErrorCode_Success) return(QString(""));
        return (QString::number(dblDouble));
    }
        break;
    case 5:
    {
        double cryCurrency = 0.0;
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(cryCurrency), intSize);
        intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &fun, &getUserData);
        if (intError != clsFileManager_intErrorCode_Success) return(QString(""));
        return (QString::number(cryCurrency));
    }
        break;
    case 6:
    {
        double dateTime = 0.0;
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(dateTime), intSize);
        intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &fun, &getUserData);
        QString strTime = QDateTime::fromMSecsSinceEpoch(dateTime).toString("yyyy-MM-dd hh:mm:ss");
        return (strTime);
    }
        break;
    case 7:
    {
        string strString = "";
        Cls_stuGetUserData getUserData(reinterpret_cast<void*&>(strString), intSize);
        int intError = Sub_FMInt->Cls_funManagerData_Extract(&dBVerify, &sDataType, &funData, &getUserData);
        if (intError != clsFileManager_intErrorCode_Success) return(QString(""));
        return (QString::fromLocal8Bit(strString.c_str()));
    }
    case 8:
        break;
    case 10:
        break;
    case 11:
        break;
    case 12:
        break;
    case 13:
        break;
    case 14:
        break;
    case 15:
        break;
    case 16:
        break;
    case 17:
        break;
    default:
        return ("");
    }
    return ("");
}

int CamelDataManager::cls_funManagerData_Delete(int nRow)
{
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    Cls_stuDataType sDType(-1, nRow+2, nullptr);
    int intError = Sub_FMInt->Cls_funManagerData_Delete(&dBVerify, &sDType);
    return(intError);
}

int CamelDataManager::cls_funManagerData_Modify(int nDataType, QString strName, QString strValue)
{
    Cls_stuDBVerify dBVerify(m_strCurDBPath.c_str(), m_strCurDBPass.c_str());
    string sName = strName.toStdString();
    Cls_stuDataType sDataType(nDataType, -1, sName.c_str());
    int intError = 0;
    Cls_lpstuUserData sUserData = nullptr;
    string strData = "";
    switch (nDataType) {
    case 1:
    {
        short sData = strValue.toShort();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 2:
    {
        int sData = strValue.toInt();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 3:
    {
        float sData = strValue.toFloat();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 4:
    {
        double sData = strValue.toDouble();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 5:
    {
        double sData = strValue.toDouble();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 6:
    {
        double sData = strValue.toDouble();
        sUserData = new Cls_stuUserData(&sData, 0);
    }
        break;
    case 7:
    {
        strData = strValue.toLocal8Bit().data();
        sUserData = new Cls_stuUserData(&strData, 0);
    }
        break;
    case 8:
    {
    }
        break;
    case 10:
    {
    }
        break;
    case 11:
    {
    }
        break;
    case 12:
    {
    }
        break;
    case 13:
    {
    }
        break;
    case 14:
    {
    }
        break;
    case 15:
    {
    }
        break;
    case 16:
    {
    }
        break;
    case 17:
    {
    }
        break;
    default:
        return (-1);
    }
    intError = Sub_FMInt->Cls_funManagerData_Modify(&dBVerify, &sDataType, sUserData, true);
    if (intError == clsFileManager_intErrorCode_Success){
        int intSize = 0;
        intError = Sub_FMInt->Cls_funManagerData_GetSize(&dBVerify, clsFileManager_intSizeType_DataSize, &sDataType, intSize);
        if (intError == clsFileManager_intErrorCode_Success){
            m_tableData->update(strName, 2, size2String(intSize));
        }
    }
    if (sUserData != nullptr){delete sUserData;}
    return(intError);
}
