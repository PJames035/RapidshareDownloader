#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    concd = 0;
    active = 0;
    fastmode = false;
    autostart = false;

    settings = new QAction(this);
    settings->setText(tr("Settings"));

    downloader = new Downloader(this);

    ui->tblDownloads->setColumnHidden(Next, true);
    ui->tblDownloads->setSortingEnabled(false);

    this->menuBar()->addAction(settings);
    connect(settings, SIGNAL(triggered()), this, SLOT(settingsMenu()));
    connect(ui->btAdd, SIGNAL(clicked()), this, SLOT(addLinksMenu()));

    connect(ui->btDel, SIGNAL(clicked()), this, SLOT(btDel_clicked()));
    connect(ui->btStart, SIGNAL(clicked()), this, SLOT(btStart_clicked()));
    connect(ui->btPause, SIGNAL(clicked()), this, SLOT(btPause_clicked()));
    connect(ui->btStop, SIGNAL(clicked()), this, SLOT(btStop_clicked()));
    connect(ui->btMoveUp, SIGNAL(clicked()), this, SLOT(btMoveUp_clicked()));
    connect(ui->btMoveDown, SIGNAL(clicked()), this, SLOT(btMoveDown_clicked()));
    connect(this, SIGNAL(startNewDownload()), this, SLOT(btStart_clicked()));
    connect(this, SIGNAL(pauseDownload(QString)), downloader, SLOT(pauseDownload(QString)));
    connect(this, SIGNAL(stopDownload(QString)), downloader, SLOT(stopDownload(QString)));

    loadSettings();
    loadLinks();
    return;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateDownload(QString filename, QString size, QString progress, QString speed, QString eta, QString status, QString next) {
    for (int i = 0; i < ui->tblDownloads->rowCount(); i++) {
        if (ui->tblDownloads->item(i, FileName)->text() == filename) {
            ui->tblDownloads->item(i, FileSize)->setText(size);
            ui->tblDownloads->item(i, Progress)->setText(progress);
            ui->tblDownloads->item(i, Speed)->setText(speed);
            ui->tblDownloads->item(i, ETA)->setText(eta);
            ui->tblDownloads->item(i, Status)->setText(status);
            ui->tblDownloads->item(i, Next)->setText(next);
            if (status == tr("COMPLETED")) {
                active--;
                emit startNewDownload();
            }
            saveLinks();
        }
    }
}

void MainWindow::btDel_clicked( void ) {
    QModelIndexList list = ui->tblDownloads->selectionModel()->selectedRows();
    sortList(list);
    int selected = list.size();
    if (selected) {
        for (int i = 0; i < selected; i++) {
            if (ui->tblDownloads->item(list.at(i).row(), Status)->text() == tr("DOWNLOADING")) {
                active--;
                emit stopDownload(ui->tblDownloads->item(list.at(i).row(), FileName)->text());
                ui->tblDownloads->removeRow(list.at(i).row());
            } else {
                ui->tblDownloads->removeRow(list.at(i).row());
            }
        }
    }
    saveLinks();
}

void MainWindow::btStart_clicked( void ) {
    if (active >= concd)
        return;

    if (sender() != this) {
        QModelIndexList list = ui->tblDownloads->selectionModel()->selectedRows();
        sortList(list);
        if (list.size()) {
            for (int i = 0; active < concd && i < list.size(); i++) {
                if (ui->tblDownloads->item(list.at(i).row(), Status)->text() == tr("WAITING")) {
                    if (downloader->download(ui->tblDownloads->item(list.at(i).row(), FileName)->text(), ui->tblDownloads->item(list.at(i).row(), Path)->text()))
                        active++;
                }
            }
            return;
        }
    }

    for (int i = 0; active < concd && i < ui->tblDownloads->rowCount(); i++) {
        if (ui->tblDownloads->item(i, Status)->text() == tr("WAITING")) {
            if (downloader->download(ui->tblDownloads->item(i, FileName)->text(), ui->tblDownloads->item(i, Path)->text())) {
                ui->tblDownloads->item(i, Status)->setText(tr("QUEUING"));
                active++;
            }
        }
    }
}

void MainWindow::btPause_clicked( void ) {
    QModelIndexList list = ui->tblDownloads->selectionModel()->selectedRows();
    sortList(list);
    int selected = list.size();
    if (selected) {
        for (int i = 0; i < selected; i++) {
            if (ui->tblDownloads->item(list.at(i).row(), Status)->text() == tr("DOWNLOADING")) {
                active--;
                emit pauseDownload(ui->tblDownloads->item(list.at(i).row(), FileName)->text());
            }
        }
    }
}

void MainWindow::btStop_clicked( void ) {
    QModelIndexList list = ui->tblDownloads->selectionModel()->selectedRows();
    sortList(list);
    int selected = list.size();
    if (selected) {
        for (int i = 0; i < selected; i++) {
            if (ui->tblDownloads->item(list.at(i).row(), Status)->text() == tr("DOWNLOADING")) {
                active--;
                emit stopDownload(ui->tblDownloads->item(list.at(i).row(), FileName)->text());
            }
        }
    }
}

void MainWindow::btMoveUp_clicked( void ) {
    QModelIndexList list = ui->tblDownloads->selectionModel()->selectedRows();
    sortList(list, false);
    int selected = list.size();
    if (selected) {
        QTableWidgetItem *temporary = NULL;
        int rowFrom = 0;
        int rowTo   = 0;
        for (int i = 0; i < selected; i++) {
            rowFrom = list.at(i).row();
            rowTo = rowFrom - 1;
            if (rowTo >= 0) {
                for (int i = 0; i < ui->tblDownloads->columnCount(); i++) {
                    temporary = ui->tblDownloads->takeItem(rowFrom, i);
                    ui->tblDownloads->setItem(rowFrom, i, ui->tblDownloads->takeItem(rowTo, i));
                    ui->tblDownloads->setItem(rowTo, i, temporary);
                }
            }
        }

        ui->tblDownloads->selectRow(rowTo);
    }
    saveLinks();
}

void MainWindow::btMoveDown_clicked( void ) {
    QModelIndexList list = ui->tblDownloads->selectionModel()->selectedRows();
    sortList(list);
    int selected = list.size();
    if (selected) {
        QTableWidgetItem *temporary = NULL;
        int rowFrom = 0;
        int rowTo   = 0;
        for (int i = 0; i < selected; i++) {
            rowFrom = list.at(i).row();
            rowTo = rowFrom + 1;
            if (rowTo <= ui->tblDownloads->rowCount() - 1) {
                for (int i = 0; i < ui->tblDownloads->columnCount(); i++) {
                    temporary = ui->tblDownloads->takeItem(rowFrom, i);
                    ui->tblDownloads->setItem(rowFrom, i, ui->tblDownloads->takeItem(rowTo, i));
                    ui->tblDownloads->setItem(rowTo, i, temporary);
                }
            }
        }
        ui->tblDownloads->selectRow(rowTo);
    }
    saveLinks();
}

void MainWindow::addLinksMenu( void ) {
    AddLinksPanel *panel = new AddLinksPanel(this);
    if (panel->exec()) {
        QStringList links = panel->getLinks()->toPlainText().split('\n');
        QString fpath = panel->getPath();
        for (int i = 0; i < links.length(); i++) {
            addLink(links.at(i), fpath);
        }
    }
}

void MainWindow::addLink( const QString &link, const QString &saveAs, const QString &statusText, const int nextText ) {
    int nextrow = ui->tblDownloads->rowCount();
    ui->tblDownloads->insertRow(nextrow);
    QTableWidgetItem *name = new QTableWidgetItem();
    QTableWidgetItem *path = new QTableWidgetItem();
    QTableWidgetItem *status = new QTableWidgetItem();
    QTableWidgetItem *size = new QTableWidgetItem();
    QTableWidgetItem *progress = new QTableWidgetItem();
    QTableWidgetItem *speed = new QTableWidgetItem();
    QTableWidgetItem *eta = new QTableWidgetItem();
    QTableWidgetItem *next = new QTableWidgetItem();

    name->setText(link);
    path->setText(saveAs);
    if (statusText.isEmpty())
        status->setText(tr("WAITING"));
    else
        status->setText(statusText);
    size->setText("");
    progress->setText("");
    speed->setText("");
    eta->setText("");
    if (nextText)
        next->setText(QString::number(nextText));
    else
        next->setText("");

    name->setTextAlignment(Qt::AlignCenter);
    path->setTextAlignment(Qt::AlignCenter);
    status->setTextAlignment(Qt::AlignCenter);
    size->setTextAlignment(Qt::AlignCenter);
    progress->setTextAlignment(Qt::AlignCenter);
    speed->setTextAlignment(Qt::AlignCenter);
    eta->setTextAlignment(Qt::AlignCenter);
    next->setTextAlignment(Qt::AlignCenter);

    ui->tblDownloads->setItem(nextrow, FileName, name);
    ui->tblDownloads->setItem(nextrow, Path, path);
    ui->tblDownloads->setItem(nextrow, Progress, progress);
    ui->tblDownloads->setItem(nextrow, FileSize, size);
    ui->tblDownloads->setItem(nextrow, Speed, speed);
    ui->tblDownloads->setItem(nextrow, ETA, eta);
    ui->tblDownloads->setItem(nextrow, Status, status);
    ui->tblDownloads->setItem(nextrow, Next, next);
}

void MainWindow::settingsMenu( void ) {
    SettingsPanel *panel = new SettingsPanel(this);
    if (panel->exec())
        loadSettings();
}

void MainWindow::saveLinks( void ) {
    QSettings settings("NoOrganization", "RapidshareDownloader");
    settings.beginGroup("Links");
    settings.remove("");
    for (int i = 0; i < ui->tblDownloads->rowCount(); i++) {
        settings.beginGroup(QString("Link #%1").arg(i));
        settings.setValue("link", ui->tblDownloads->item(i, FileName)->text());
        settings.setValue("status", ui->tblDownloads->item(i, Status)->text());
        settings.setValue("path", ui->tblDownloads->item(i, Path)->text());
        settings.setValue("next", ui->tblDownloads->item(i, Next)->text());
        settings.endGroup();
    }
    settings.endGroup();
}

void MainWindow::loadLinks( void ) {
    ui->tblDownloads->clear();
    ui->tblDownloads->setRowCount(0);
    QSettings settings("NoOrganization", "RapidshareDownloader");
    settings.beginGroup("Links");
    for (int i = 0;; i++) {
        settings.beginGroup(QString("Link #%1").arg(i));
        if (settings.value("link").toString() == "") {
            settings.endGroup();
            break;
        }
        addLink(settings.value("link").toString(), settings.value("path").toString(), settings.value("status").toString(), settings.value("next").toInt());
        settings.endGroup();
    }
    settings.endGroup();
}

void MainWindow::loadSettings( void ) {
    QSettings settings("NoOrganization", "RapidshareDownloader");
    settings.beginGroup("Settings");
    concd = settings.value("concd").toInt();
    fastmode = settings.value("fastmode").toBool();
    autostart = settings.value("autostart").toBool();
    settings.endGroup();
    return;
}

void MainWindow::resizeEvent(QResizeEvent *e) {
    QMainWindow::resizeEvent(e);
    if (!(e->size().height() < 0)) {
        ui->layoutActions->setGeometry(QRect(0, 0, ui->layoutActions->geometry().width(), ui->layoutActions->geometry().height()));
        ui->tblDownloads->setGeometry(QRect(0, ui->layoutActions->geometry().height() + 10, width(), height() - ui->layoutActions->geometry().height() - 10));
    }
    return;
}

void MainWindow::sortList(QModelIndexList &list, bool ascending) {
    for (int i = 0; i < list.size(); i++) {
        for (int j = 0; j < list.size() - 1; j++) {
            if (ascending) {
                if (list.at(j).row() < list.at(j + 1).row())
                    list.move(j, j + 1);
            } else {
                if (list.at(j).row() > list.at(j + 1).row())
                    list.move(j, j + 1);
            }
        }
    }
}
