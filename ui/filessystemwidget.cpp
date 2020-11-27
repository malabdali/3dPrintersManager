#include "filessystemwidget.h"
#include "ui_filessystemwidget.h"
#include <QFileDialog>
#include <QPair>
#include "../core/devicefilessystem.h"
#include "../core/utilities/loadfilefuture.h"
#include "../core/gcode/startprinting.h"
#include "../core/fileinfo.h"

FilesSystemWidget::FilesSystemWidget(Device* device,QWidget *parent) :
    QWidget(parent),_device(device),
    ui(new Ui::FilesSystemWidget)
{
    ui->setupUi(this);
    QObject::connect(_device->GetFileSystem(),&DeviceFilesSystem::FileListUpdated,this,&FilesSystemWidget::OnFileListUpdated);
    QObject::connect(_device->GetFileSystem(),&DeviceFilesSystem::UploadFileFailed,this,&FilesSystemWidget::OnFileListUpdated);
    ui->_delete_file_button->setVisible(false);
    ui->_print_button->setVisible(false);
    OnFileListUpdated();
}

FilesSystemWidget::~FilesSystemWidget()
{
    delete ui;
}

void FilesSystemWidget::on__update_files_button_clicked()
{
    this->_device->GetFileSystem()->UpdateFileList();
}

void FilesSystemWidget::OnFileListUpdated()
{
    ui->_files_list->clear();
    auto files=_device->GetFileSystem()->GetFileList();
    for(int i=0;i<files.size();i++){
        QListWidgetItem* lwi=new QListWidgetItem(ui->_files_list);
        lwi->setIcon(QIcon(":/icon/images/file.png"));
        lwi->setText(QString(files[i].GetFileName())+" : "+QString::number(files[i].GetLocaleFileInfo().size()/1000.0,'f',2)+" : "+files[i].GetLocaleFileInfo().birthTime().toString());
        lwi->setData(0x0100,files[i].GetFileName());
        lwi->setData(3,"uploaded");
        ui->_files_list->addItem(lwi);
    }
    auto wfiles=_device->GetFileSystem()->GetWaitUploadingList();
    for(int i=0;i<wfiles.size();i++){
        QListWidgetItem* lwi=new QListWidgetItem(ui->_files_list);
        lwi->setIcon(QIcon(":/icon/images/uploading_file.png"));
        lwi->setText(wfiles[i]+"*");
        lwi->setData(0x0100,wfiles[i]);
        lwi->setData(3,"waiting");
        ui->_files_list->addItem(lwi);
    }

    auto ffiles=_device->GetFileSystem()->GetFailedUploads();
    for(int i=0;i<ffiles.size();i++){
        QListWidgetItem* lwi=new QListWidgetItem(ui->_files_list);
        lwi->setIcon(QIcon(":/icon/images/fail.png"));
        lwi->setText(ffiles[i]+"*");
        lwi->setData(0x0100,ffiles[i]);
        lwi->setData(3,"failed");
        ui->_files_list->addItem(lwi);
    }
}

void FilesSystemWidget::on__files_list_itemSelectionChanged()
{
    ui->_delete_file_button->setVisible(false);
    ui->_stop_upload_button->setVisible(false);
    ui->_print_button->setVisible(false);
    if(ui->_files_list->selectedItems().length()>0){
        for(QListWidgetItem* &item:ui->_files_list->selectedItems()){
            QString name=item->data(0x0100).toString();
            if(item->data(3)=="uploaded"){
                ui->_delete_file_button->setVisible(true);
                ui->_print_button->setVisible(true);
            }
            else if(item->data(3)=="waiting"){
                ui->_stop_upload_button->setVisible(true);
            }
            else if(item->data(3)=="failed"){
            }
        }
    }
}

void FilesSystemWidget::on__delete_file_button_clicked()
{
    ui->_delete_file_button->setVisible(false);
    ui->_print_button->setVisible(false);
    for(QListWidgetItem* &item:ui->_files_list->selectedItems()){
        QString name=item->data(0x0100).toString();
        if(_device->GetFileSystem()->GetFileList().contains(FileInfo(name.toUtf8()))){
            _device->GetFileSystem()->DeleteFile(name.toUtf8());
        }
    }

}

void FilesSystemWidget::on__upload_file_button_clicked()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::FileMode::ExistingFiles);
    fd.setNameFilter("*.GCO *.gcode *.G");
    if(fd.exec()){
        if(fd.selectedFiles().length()>0)
        {
            for(QString f:fd.selectedFiles())
            {
                QByteArray fileName=QUrl(f).fileName().replace("gcode",UPLOAD_SUFFIX).toUpper().toUtf8();
                if(!ui->_overwrite_checkbox->isChecked() && _device->GetFileSystem()->GetFileList().contains(FileInfo(fileName)))
                    continue;
                _device->GetFileSystem()->UploadFile(f.toUtf8());
            }
        }
    }
}


void FilesSystemWidget::on__stop_upload_button_clicked()
{
    //qDebug()<<QThread::currentThread()<<_device->thread();
    _device->PauseCommands();
    QList<QString> names;
    for(QListWidgetItem* item:ui->_files_list->selectedItems()){
        QString name=item->data(0x0100).toString();
        names.append(name);
    }
    qDebug()<<names;
    for(QString& name:names){
        if(_device->GetFileSystem()->GetWaitUploadingList().contains(name.toUtf8())){
            _device->GetFileSystem()->StopUpload(name.toUtf8());
        }
    }
    _device->PlayCommands();

    OnFileListUpdated();
}

void FilesSystemWidget::on__print_button_clicked()
{
    ui->_delete_file_button->setVisible(false);
    ui->_print_button->setVisible(false);
    QListWidgetItem* item=ui->_files_list->selectedItems()[0];
    QString name=item->text().mid(0,item->text().indexOf(" "));
    _device->AddGCodeCommand(new GCode::StartPrinting(_device,name.toUtf8()));

}