#include "settingspanel.h"

SettingsPanel::SettingsPanel(QWidget *parent) :
    QDialog(parent)
{
    lbSettings = new QLabel(this);
    lbRsHeader = new QLabel(this);
    lbRsUser = new QLabel(this);
    lbRsPass = new QLabel(this);
    lbRsChecked = new QLabel(this);
    lbConcDownloads = new QLabel(this);

    cbFastMode = new QCheckBox(this);
    cbAutoStart = new QCheckBox(this);
    cbStartMinimized = new QCheckBox(this);

    tbRsUser = new QLineEdit(this);
    tbRsPass = new QLineEdit(this);
    tbConcDownloads = new QLineEdit(this);

    btOk = new QPushButton(this);
    btCancel = new QPushButton(this);
    btRsCheckAccount = new QPushButton(this);

    layoutRs = new QGridLayout();
    layoutSettings = new QBoxLayout(QBoxLayout::Down);
    layoutHButtons = new QBoxLayout(QBoxLayout::LeftToRight);

    tblN2P = new QTableWidget(0, 3);

    addRow = new QAction(tr("Insert Row"), tblN2P);
    delRow = new QAction(tr("Remove Row"), tblN2P);

    lbSettings->setText(tr("Settings"));
    lbRsHeader->setText(tr("Rapidshare Premium"));
    lbRsUser->setText(tr("Username"));
    lbRsPass->setText(tr("Password"));
    lbRsChecked->setText(tr("Not Checked"));
    lbConcDownloads->setText(tr("Concurrent Downloads"));

    cbFastMode->setText(tr("Fast-Mode"));
    cbAutoStart->setText(tr("AutoStart"));
    cbStartMinimized->setText(tr("Start Minimized"));

    lbRsHeader->setAlignment(Qt::AlignHCenter);
    lbRsUser->setAlignment(Qt::AlignHCenter);
    lbRsPass->setAlignment(Qt::AlignHCenter);
    lbRsChecked->setAlignment(Qt::AlignHCenter);

    lbRsChecked->setTextFormat(Qt::RichText);

    btOk->setText(tr("OK"));
    btCancel->setText(tr("Cancel"));
    btRsCheckAccount->setText(tr("Check Account"));

    layoutRs->addWidget(lbRsHeader, 0, 0);
    layoutRs->addWidget(lbRsUser, 1, 0);
    layoutRs->addWidget(lbRsPass, 2, 0);
    layoutRs->addWidget(lbRsChecked, 3, 0);
    layoutRs->addWidget(tbRsUser, 1, 1);
    layoutRs->addWidget(tbRsPass, 2, 1);
    layoutRs->addWidget(btRsCheckAccount, 3, 1);
    layoutRs->addWidget(lbConcDownloads, 1, 4);
    layoutRs->addWidget(tbConcDownloads, 1, 5);
    layoutRs->addWidget(cbFastMode, 2, 4);
    layoutRs->addWidget(cbAutoStart, 3, 4);
    layoutRs->addWidget(cbStartMinimized, 2, 5);

    layoutHButtons->addWidget(btOk);
    layoutHButtons->addWidget(btCancel);

    layoutSettings->addWidget(lbSettings, 1, Qt::AlignHCenter);
    layoutSettings->addLayout(layoutRs);
    layoutSettings->addWidget(tblN2P);
    layoutSettings->addLayout(layoutHButtons);

    this->setLayout(layoutSettings);
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    tblN2P->addAction(addRow);
    tblN2P->addAction(delRow);
    tblN2P->setContextMenuPolicy(Qt::ActionsContextMenu);
    tblN2P->setItemDelegate(new ComboBoxDelegate(0));
    tblN2P->setHorizontalHeaderLabels(QStringList() << tr("Order") << tr("Tag") << tr("Suggested Path"));
    connect(tblN2P, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(findPathForCell(int, int)));

    connect(btOk, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(btCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(addRow, SIGNAL(triggered()), this, SLOT(insertRow()));
    connect(delRow, SIGNAL(triggered()), this, SLOT(removeRow()));
    connect(btRsCheckAccount, SIGNAL(clicked()), this, SLOT(checkPremium()));

    this->setTabOrder(tbRsUser, tbRsPass);
    this->setTabOrder(tbRsPass, btRsCheckAccount);
    this->setTabOrder(btRsCheckAccount, tbConcDownloads);
    this->setTabOrder(tbConcDownloads, cbFastMode);
    this->setTabOrder(cbFastMode, cbAutoStart);
    this->setTabOrder(cbAutoStart, cbStartMinimized);
    this->setTabOrder(cbStartMinimized, tblN2P);
    this->setTabOrder(tblN2P, btOk);
    this->setTabOrder(btOk, btCancel);

    loadSettings();
    this->setWindowTitle("Settings");
    return;
}

void SettingsPanel::insertRow( void ) {
    tblN2P->insertRow(tblN2P->rowCount());
}

void SettingsPanel::removeRow( void ) {
    tblN2P->removeRow(tblN2P->currentRow());
}

void SettingsPanel::checkPremium( void ) {
    Downloader *downloader = new Downloader(this);
    if (downloader->checkAccount(tbRsUser->text(), tbRsPass->text()))
        lbRsChecked->setText("<font color=\"#40FF00\">Valid!</font>");
    else
        lbRsChecked->setText("<font color=\"#FF0000\">Invalid!</font>");
}

void SettingsPanel::findPathForCell(int row, int column) {
    if (column == 2) {
        QTableWidgetItem *item = new QTableWidgetItem(QFileDialog::getExistingDirectory(this, tr("Locate destination directory"), "/"));
        tblN2P->setItem(row, column, item);
    }
}

void SettingsPanel::saveSettings( void ) {
    QSettings settings("NoOrganization", "RapidshareDownloader");
    settings.beginGroup("Settings");
    settings.remove("");
    settings.setValue("rsuser", tbRsUser->text());
    settings.setValue("rspass", tbRsPass->text());
    settings.setValue("concd", tbConcDownloads->text());
    settings.setValue("fastmode", cbFastMode->isChecked() ? "true" : "false");
    settings.setValue("autostart", cbAutoStart->isChecked() ? "true" : "false");
    settings.setValue("startminimized", cbStartMinimized->isChecked() ? "true" : "false");
    for (int row = 0; row < tblN2P->rowCount(); row++) {
        settings.beginGroup(QString("Preference #%1").arg(row));
        settings.remove("");
        settings.setValue("1", tblN2P->item(row, 0)->data(Qt::DisplayRole).toString());
        settings.setValue("2", tblN2P->item(row, 1)->text());
        settings.setValue("3", tblN2P->item(row, 2)->text());
        settings.endGroup();
    }
    settings.endGroup();
    accept();
}

void SettingsPanel::loadSettings( void ) {
    QSettings settings("NoOrganization", "RapidshareDownloader");
    settings.beginGroup("Settings");
    tbRsUser->setText(settings.value("rsuser").toString());
    tbRsPass->setText(settings.value("rspass").toString());
    tbConcDownloads->setText(settings.value("concd").toString());
    cbFastMode->setChecked(settings.value("fastmode").toBool());
    cbAutoStart->setChecked(settings.value("autostart").toBool());
    cbStartMinimized->setChecked(settings.value("startminimized").toBool());
    for (int currentPref = 0;; currentPref++) {
        settings.beginGroup(QString("Preference #%1").arg(currentPref));
        if (settings.value("1").isNull()) {
            settings.endGroup();
            break;
        }
        tblN2P->insertRow(currentPref);

        QTableWidgetItem *item0 = new QTableWidgetItem(settings.value("1").toString());
        QTableWidgetItem *item1 = new QTableWidgetItem(settings.value("2").toString());
        QTableWidgetItem *item2 = new QTableWidgetItem(settings.value("3").toString());
        tblN2P->setItem(currentPref, Order, item0);
        tblN2P->setItem(currentPref, Tag, item1);
        tblN2P->setItem(currentPref, Path, item2);

        settings.endGroup();
    }
    settings.endGroup();
}
