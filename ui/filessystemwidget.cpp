#include "filessystemwidget.h"
#include "ui_filessystemwidget.h"
#include <QFileDialog>
#include <QPair>
#include "../core/devicefilessystem.h"
#include "../core/utilities/loadfilefuture.h"

FilesSystemWidget::FilesSystemWidget(Device* device,QWidget *parent) :
    QWidget(parent),_device(device),
    ui(new Ui::FilesSystemWidget)
{
    ui->setupUi(this);
    QObject::connect(_device->GetFileSystem(),&DeviceFilesSystem::FileListUpdated,this,&FilesSystemWidget::OnFileListUpdated);
    QObject::connect(_device->GetFileSystem(),&DeviceFilesSystem::UploadFileFailed,this,&FilesSystemWidget::OnFileListUpdated);
    ui->_delete_file_button->setVisible(false);
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
        lwi->setText(QString(files.keys()[i]+" : "+QByteArray::number(files.values()[i])));
        ui->_files_list->addItem(lwi);
    }
    auto wfiles=_device->GetFileSystem()->GetWaitUploadingList();
    for(int i=0;i<wfiles.size();i++){
        QListWidgetItem* lwi=new QListWidgetItem(ui->_files_list);
        lwi->setIcon(QIcon(":/icon/images/uploading_file.png"));
        lwi->setText(wfiles[i]+"*");
        ui->_files_list->addItem(lwi);
    }

    auto ffiles=_device->GetFileSystem()->GetFailedUploads();
    for(int i=0;i<ffiles.size();i++){
        QListWidgetItem* lwi=new QListWidgetItem(ui->_files_list);
        lwi->setIcon(QIcon(":/icon/images/fail.png"));
        lwi->setText(ffiles[i]+"*");
        ui->_files_list->addItem(lwi);
    }
}

void FilesSystemWidget::on__files_list_itemSelectionChanged()
{
    ui->_delete_file_button->setVisible(false);
    ui->_stop_upload_button->setVisible(false);
    if(ui->_files_list->selectedItems().length()>0){
        for(QListWidgetItem* &item:ui->_files_list->selectedItems()){
            QString name=item->text().mid(0,item->text().indexOf(" "));
            if(_device->GetFileSystem()->GetFileList().contains(name.toUtf8())){
                ui->_delete_file_button->setVisible(true);
            }
            else if(name.indexOf("*")){
                if(_device->GetFileSystem()->GetWaitUploadingList().contains(name.mid(0,name.length()-1).toUtf8()))
                {
                    ui->_stop_upload_button->setVisible(true);
                }
            }
        }
    }
}

void FilesSystemWidget::on__delete_file_button_clicked()
{
    ui->_delete_file_button->setVisible(false);
    for(QListWidgetItem* &item:ui->_files_list->selectedItems()){
        QString name=item->text().mid(0,item->text().indexOf(" "));
        if(_device->GetFileSystem()->GetFileList().contains(name.toUtf8())){
            _device->GetFileSystem()->DeleteFile(name.toUtf8());
        }
    }

}

void FilesSystemWidget::on__upload_file_button_clicked()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::FileMode::ExistingFiles);
    fd.setNameFilter("*.GCO *.gcode");
    if(fd.exec()){
        if(fd.selectedFiles().length()>0)
        {
            for(QString f:fd.selectedFiles())
            {
                QByteArray fileName=QUrl(f).fileName().replace("gcode","GCO").toUpper().toUtf8();
                if(!ui->_overwrite_checkbox->isChecked() && _device->GetFileSystem()->GetFileList().contains(fileName))
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
        QString name=item->text().mid(0,item->text().indexOf(" "));
        names.append(name);
    }
    qDebug()<<names;
    for(QString& name:names){
        if(_device->GetFileSystem()->GetWaitUploadingList().contains(name.toUtf8().mid(0,name.length()-1))){
            _device->GetFileSystem()->StopUpload(name.toUtf8().mid(0,name.length()-1));
        }
    }
    _device->PlayCommands();

    OnFileListUpdated();
}
