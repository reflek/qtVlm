/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#include <QDebug>

#include "opponentBoat.h"

#include "boatAccount.h"
#include "MainWindow.h"

/****************************************
* Opponent methods
****************************************/

opponent::opponent(QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,QWidget *main, QWidget *parentWindow):QWidget(parentWindow)
{
    init(false,idu,race,lat,lon,login,name,proj,main,parentWindow);    
}

opponent::opponent(QString idu,QString race,Projection * proj,QWidget *main, QWidget *parentWindow):QWidget(parentWindow)
{
    init(true,idu,race,0,0,"","",proj,main,parentWindow);    
}

void opponent::init(bool isQtBoat,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,QWidget *main, QWidget *parentWindow)
{
    this->idu=idu;
    this->idrace=race;
    this->lat=lat;
    this->lon=lon;
    this->login=login;
    this->name=name;
    this->proj=proj;
    
    this->isQtBoat = isQtBoat;
    
    createWidget();
    updatePosition();
    paramChanged();
    
    connect(parentWindow, SIGNAL(projectionUpdated()), this, SLOT(updateProjection()));
    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(paramChanged()));
    
    if(!isQtBoat)
        show();
}

void opponent::createWidget(void)
{ 
    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    QGridLayout *layout = new QGridLayout();
    layout->setContentsMargins(10,0,2,0);
    layout->setSpacing(0);

    label = new QLabel(login, this);
    label->setFont(QFont("Helvetica",9));

    QPalette p;
    p.setBrush(QPalette::Active, QPalette::WindowText, fgcolor);
    p.setBrush(QPalette::Inactive, QPalette::WindowText, fgcolor);
    label->setPalette(p);
    label->setAlignment(Qt::AlignHCenter);

    layout->addWidget(label, 0,0, Qt::AlignHCenter|Qt::AlignVCenter);
    this->setLayout(layout);
    setAutoFillBackground (false);
}

void  opponent::paintEvent(QPaintEvent *)
{
    if(isQtBoat)
        return;
    QPainter pnt(this);
    int dy = height()/2;

    pnt.fillRect(9,0, width()-10,height()-1, QBrush(bgcolor));

    QPen pen(myColor);
    pen.setWidth(4);
    pnt.setPen(pen);
    pnt.fillRect(0,dy-3,7,7, QBrush(myColor));

    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt.setPen(pen);
    pnt.drawRect(9,0,width()-10,height()-1);
}

void opponent::updateProjection()
{
    updatePosition();
    update();
}

void opponent::updatePosition()
{    
    int boat_i,boat_j;

    Util::computePos(proj,lat,lon,&boat_i,&boat_j);
    boat_i-=3;
    boat_j-=(height()/2);
    
    qWarning() << name << ": at (" << boat_i << "," << boat_j << ") => " << lat << "," << lon;
    
    move(boat_i, boat_j);    
}

void opponent::setName()
{
    this->name=name;
    QString str;
    QString str2;
    switch(label_type)
    {
        case OPP_SHOW_LOGIN:
            str = login;
            str2 = idu + " - " + name;
            break;
        case OPP_SHOW_NAME:
            str = name;
            str2 = idu + " - " + login;
            break;
        case OPP_SHOW_IDU:
            str = idu;
            str2 = login + " - " + name;
            break;        
    }
    label->setText(str);
    setToolTip(str2);
    adjustSize();
}

void opponent::setNewData(float lat, float lon,QString name)
{  
    bool needUpdate = false;
    
    if(lat != this->lat || lon != this->lon)
    {
       this->lat=lat;
       this->lon=lon;
       updatePosition();
       needUpdate = true;       
    }
        
    if(name != this->name)
    {
        this->name=name;
        setName();
        needUpdate = true;
    }
        
    /* new data => we are not a qtVlm boat */
    if(isQtBoat)
    {
        setIsQtBoat(false);
        #warning why doing this ?
    }
    else
    {
        if(needUpdate)
            update();
    }
}

void opponent::setIsQtBoat(bool status)
{
    if(status == isQtBoat)
        return;
    isQtBoat=status;
    if(isQtBoat)
        hide();
    else
        show();
}

void opponent::paramChanged()
{
    myColor = QColor(Util::getSetting("opp_color",QColor(Qt::green).name()).toString());
    label_type = Util::getSetting("opp_labelType",0).toInt();
    setName();
    if(isQtBoat)
        update();
}

/****************************************
* Opponent list methods
****************************************/
#define OPP_TYPE_POSITION  0
#define OPP_TYPE_NAME      1

#define OPP_NO_REQUEST     0
#define OPP_BOAT_DATA      1
#define OPP_BOAT_TRJ       2

#define OPP_MODE_REFRESH   0
#define OPP_MODE_NEWLIST   1

opponentList::opponentList(Projection * proj,MainWindow * mainWin,QWidget *parentWindow) : QObject(parentWindow)
{
    parent=parentWindow;
    this->mainWin=mainWin;
    this->proj=proj;
    /* init http inetManager */
    inetManager = new QNetworkAccessManager(this);
    currentRequest=OPP_NO_REQUEST;
    if(inetManager)
    {
        host = Util::getHost();
        connect(inetManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished (QNetworkReply*)));
        Util::paramProxy(inetManager,host);
    }
}

QString opponentList::getRaceId()
{
    if(opponent_list.size()<=0)
        return "";
    
    return opponent_list[0]->getRace();
}

void opponentList::setBoatList(QString list_txt,QString race,bool force)
{
    if(currentRequest!=OPP_NO_REQUEST)
    {
        qWarning() << "getOpponents request still running";
        return;
    }
    
    /* is a list defined ? */
    if(opponent_list.size()>0)
    {        
        if(!force && opponent_list[0]->getRace() == race) /* it is the same race */
        { /* compare if same opp list */
            /* for now it is the same => refresh*/
            refreshData();
            return;
        }
        /* clear current list */
        clear();
    }
    
    currentRequest=OPP_BOAT_DATA;
    currentOpponent = 0;
    currentList = list_txt.split(";");
    currentRace = race;
    currentMode = OPP_MODE_NEWLIST;
    
    if(currentList.size() > 0)
        getNxtOppData();
    else
        currentRequest=OPP_NO_REQUEST;
}

void opponentList::clear(void)
{
       if(opponent_list.size() > 0)
       {
              for(int i=0;i<opponent_list.size();i++)
                  delete opponent_list[i];
              opponent_list.clear();
       }   
}

void opponentList::refreshData(void)
{
    if(currentRequest!=OPP_NO_REQUEST)
    {
        qWarning() << "getOpponents request still running";
        return;
    }
    
    if(opponent_list.size()<=0)
        return;
    
    currentRequest=OPP_BOAT_DATA;
    currentRace = opponent_list[0]->getRace();
    currentOpponent = 0;
    currentMode=OPP_MODE_REFRESH;
    getNxtOppData();
}

void opponentList::getNxtOppData()
{
    int listSize = (currentMode==OPP_MODE_REFRESH?opponent_list.size():currentList.size());
    QString idu;
    
    if(currentOpponent>=listSize)
    {
        currentRequest=OPP_NO_REQUEST;
        return;
    }
    
    idu = (currentMode==OPP_MODE_REFRESH?opponent_list[currentOpponent]->getIduser():currentList[currentOpponent]);
    
    if(mainWin->isBoat(idu))
    {
        if(currentMode==OPP_MODE_REFRESH)
            opponent_list[currentOpponent]->setIsQtBoat(true);
        else
            opponent_list.append(new opponent(idu,currentRace,proj,mainWin,parent));
            
        currentOpponent++;
        getNxtOppData();
        return;
    }
    
    QString page;
    QTextStream(&page) << host
                        << "/gmap/index.php?"
                        << "type=ajax&riq=pos"
                        << "&idusers="
                        << idu
                        << "&idraces="
                        << currentRace;
    currentOpponent++;  
    //qWarning() << "ask for " << page;
    
    QNetworkRequest request;
    request.setUrl(QUrl(page));
    Util::addAgent(request);   
    inetManager->get(request);
}

void opponentList::requestFinished ( QNetworkReply* inetReply)
{
    if (inetReply->error() != QNetworkReply::NoError) {
        qWarning() << "Error doing inet-Get:" << inetReply->error();
        currentRequest=OPP_NO_REQUEST;
    }
    else
    {
         QString strbuf = inetReply->readAll();
         QStringList list_res;
         QStringList lsval,lsval2;
         float lat,lon;
         QString login,name;
         QString idu;
         switch(currentRequest)
         {
             case OPP_NO_REQUEST:
                 return;
             case OPP_BOAT_DATA:
                 list_res=readData(strbuf,OPP_TYPE_NAME);
                 if(list_res.size()>0)
                 {
                     /* only one data should be returned */
                     lsval=list_res[0].split(",");
                     list_res=readData(strbuf,OPP_TYPE_POSITION);
                     lsval2=list_res[0].split(",");
                     if (lsval2.size() == 2)
                     {
                         lat=lsval2[0].toFloat();
                         lon=lsval2[1].toFloat();
                         login=lsval[0].mid(4,lsval[0].size()-4-4);
                         name=lsval[1].mid(5,lsval[1].size()-5-1);
                         if(lat!=0 && lon !=0)
                         {
                             idu = (currentMode==OPP_MODE_REFRESH?
                                 opponent_list[currentOpponent-1]->getIduser():currentList[currentOpponent-1]);
                             /*qWarning() << login << "-" << name 
                                 << " at (" << lat << "," << lon << ") - idu"
                                 << idu ;*/
                             if(currentMode==OPP_MODE_REFRESH)
                                 opponent_list[currentOpponent-1]->setNewData(lat,lon,name);
                             else                                    
                                 opponent_list.append(new opponent(idu,currentRace,
                                                                lat,lon,login,name,proj,mainWin,parent));
                         }
                     }
                 }
                 getNxtOppData();
                 break;
             case OPP_BOAT_TRJ:
                 
                 break;
         }
    }
}

QStringList opponentList::readData(QString in_data,int type)
{
    QString begin_str;
    QString end_str = ")";
    if(type==OPP_TYPE_POSITION)
        begin_str = "GLatLng(";
    else
        begin_str = "openInfoWindowHtml(";
    QString sub_str;
    QStringList lst;
    bool stop=false;
    int pos=0;
    int end;
    while(!stop)
    {
        pos = in_data.indexOf(begin_str,pos);
        if(pos==-1)
            stop=true;
        else
        {
            pos+=begin_str.size();
            end = in_data.indexOf(end_str,pos);
            if(end==-1)
                continue;
            lst.append(QString(in_data.mid(pos,end-pos)));
            pos=end;
        }
    }
    return lst;
}