#include "filessystemwidget.h"
#include "ui_filessystemwidget.h"
#include <QFileDialog>
#include <QPair>

FilesSystemWidget::FilesSystemWidget(Device* device,QWidget *parent) :
    QWidget(parent),_device(device),
    ui(new Ui::FilesSystemWidget)
{
    ui->setupUi(this);
    QObject::connect(_device->GetFileSystem(),&DeviceFilesSystem::FileListUpdated,this,&FilesSystemWidget::OnFileListUpdated);
    ui->_delete_file_button->setVisible(false);
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
    qDebug()<<"files list updated";
    ui->_files_list->clear();
    auto files=_device->GetFileSystem()->GetFileList();
    for(int i=0;i<files.size();i++){
        QListWidgetItem* lwi=new QListWidgetItem(ui->_files_list);
        lwi->setText(QString(files.keys()[i]+" : "+QByteArray::number(files.values()[i])));
        ui->_files_list->addItem(lwi);
    }
}

void FilesSystemWidget::on__files_list_itemSelectionChanged()
{
    qDebug()<<"item selected";
    if(ui->_files_list->selectedItems().length()>0){
        ui->_delete_file_button->setVisible(true);
    }
    else
    {
        ui->_delete_file_button->setVisible(false);
    }
}

void FilesSystemWidget::on__delete_file_button_clicked()
{
    ui->_delete_file_button->setVisible(false);
    for(QListWidgetItem* &item:ui->_files_list->selectedItems()){
        QString name=item->text().mid(0,item->text().indexOf(" "));
        _device->GetFileSystem()->DeleteFile(name.toUtf8());
    }

}

void FilesSystemWidget::on__upload_file_button_clicked()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::FileMode::ExistingFile);
    fd.setNameFilter("*.GCO *.gcode");
    if(fd.exec()){
        if(fd.selectedFiles().length()>0)
        {
            QFile file(fd.selectedFiles()[0]);
            qDebug()<<QUrl(file.fileName()).fileName();
            file.open(QIODevice::ReadOnly);
            QByteArray ba=file.readAll();
            _device->GetFileSystem()->UploadFile(QUrl(file.fileName()).fileName().replace("gcode","GCO").toUpper().toUtf8(),ba);
        }
    }
}
