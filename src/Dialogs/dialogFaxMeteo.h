#ifndef DIALOGFAXMETEO_H
#define DIALOGFAXMETEO_H

#include <QDialog>
#include "class_list.h"
#include "faxMeteo.h"
#include "mycentralwidget.h"
#include "ui_dialogFaxMeteo.h"

class dialogFaxMeteo : public QDialog, public Ui::dialogFaxMeteo_ui
{ Q_OBJECT
    
public:
    dialogFaxMeteo(faxMeteo * fax, myCentralWidget * parent);
    ~dialogFaxMeteo();
    void done(int result);
public slots:
    void browseFile();
private:
    faxMeteo * fax;
    myCentralWidget * parent;
};

#endif // DIALOGFAXMETEO_H
