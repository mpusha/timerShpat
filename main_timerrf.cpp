/*!
*  \file main_timerrf.cpp
*  \brief Файл с реализацией класса TTimerRf.
*/
#include "main_timerrf.h"

/*!
 * @brief Конструктор класса TTimerRf.
 *
 * Создает объект с GUI пользовательски интерфейсом
 * и объект THwBehave.
 */
TTimerRf::TTimerRf(QWidget *parent)  : QMainWindow(parent)
{
  create_ListWidget();
  create_Menu();
  create_StatusBar();

  QApplication::processEvents();

  //... Text Codec ...
  QTextCodec *russianCodec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForLocale(russianCodec);

  setWindowTitle(tr("Program for setup RF timer"));

  modifyData=false;
  dev=new THwBehave;
  connect(dev, SIGNAL(signalMsg(QString,int)), this, SLOT(slot_processMsg(QString,int)));
  connect(dev, SIGNAL(signalDataReady(int)), this, SLOT(slot_processData(int)));
  setMinimumSize(900,320);
  //showMaximized();
  resize(900,320);
}

/*!
 * @brief Деструктор класса TTimerRf.
 *
 * Удаляет созданные объекты таблицы и объект THwBehave.
 */
TTimerRf::~TTimerRf()
{    
  for(int i=0;i<8;i++) {
    delete itemTable[0][i];
  }
  delete dev;
}

//-----------------------------PUBLIC METHODS
/*!
 * @brief Занесение данных в поля таблицы.
 *
 * Запись данных в поля таблицы. Данные о времени срабатывания каналов
 * представляются в виде числа в мс с двумя значащими числами после запятой.
 */
void TTimerRf::putDataToTable(void)
{
  for(int i=0;i<ALLVECTORS;i++) {
    data[0][i]=round(data[0][i]*100)/100.0;
    itemTable[0][i]->setText(QString("%1").arg(data[0][i],5,'f',2));
  }
}

/*!
 * @brief Считывание данных из полей таблицы.
 *
 * Считывание данных из полей таблицы во внутреннюю переменную data[]. Данные о времени срабатывания каналов
 * представляются в виде числа в мс с двумя значащими числами после запятой. Усли данные <0, записывается 0.
 * Если данные > #maxTime записывается #maxTime.
 */
void TTimerRf::getDataFromTable(void)
{
  bool ok;
  double tmp;
  for(int i=0;i<ALLVECTORS;i++) {
    tmp=itemTable[0][i]->text().toDouble(&ok);
    if(tmp<0) tmp=0;
    else if(tmp>maxTime)
      tmp=maxTime;
    tmp=round(tmp*100);
    data[0][i]=tmp/100.0;
    if(!ok) data[0][i]=0;
  }
}
//------------------------------------PRIVATE METHODS
/*!
 * @brief Создание виджетов в пользовательском окне.
 *
 * Создает виджеты меню, таблица, кнопки, статусная строка
 * в окне пользовательского интерфейса.
 */
void TTimerRf::create_ListWidget()
{
  MainGroupBox = new QGroupBox(tr("Prepare data"),this);
  MainGroupBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
  setCentralWidget(MainGroupBox);
  main_layout = new QVBoxLayout();
  main_layout->setMargin(5);
  main_layout->setSpacing(4);

  update_btn=new QPushButton(tr("Update"),this);
  write_btn=new QPushButton(tr("Write"),this);

  edit_layout = new QHBoxLayout;

  edit_layout->addWidget(update_btn); edit_layout->addStretch(1);
  edit_layout->addWidget(write_btn); edit_layout->addStretch(1);

  edit_layout->setSpacing(10);

  data_layout=new QHBoxLayout;

  tableRf=new QTableWidget(this);
  //tableRf->verticalHeader()->hide();
  tableRf->setRowCount(1);
  tableRf->setColumnCount(ALLVECTORS);
  //QTableWidgetItem *labelItem1= new QTableWidgetItem(tr("time, us"));labelItem1->setBackgroundColor(HEADER_COLOR); tableRf->setItem(0,0,labelItem1);
  QStringList headerV,headerH;
  headerH<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8";
  headerV<<tr("time, ms");
  tableRf->setHorizontalHeaderLabels(headerH);
  tableRf->setVerticalHeaderLabels(headerV);
  tableRf->horizontalHeader()->setStretchLastSection(true);
  //tableRf->verticalHeader()->setStretchLastSection(false);
  //tableRf->setMinimumHeight(50);
  //tableRf->setMaximumHeight(50);
  //for(int i=1;i<=ALLVECTORS;i++)
  //  tableRf->setColumnWidth(i,200);
  //tableRf->resizeColumnsToContents();
  data_layout->addWidget(tableRf);


  main_layout->addLayout(data_layout);

  main_layout->addLayout(edit_layout);

  MainGroupBox->setLayout(main_layout);

// create table items
 for(int i=0;i<ALLVECTORS;i++) {
   itemTable[0][i]=new QTableWidgetItem();
   tableRf->setItem(0,i,itemTable[0][i]);

   tableRf->setColumnWidth(i,80);
 }
 putDataToTable();
 connect(update_btn,SIGNAL(pressed()), this, SLOT(slot_updateHW()));
 connect(write_btn,SIGNAL(pressed()), this, SLOT(slot_writeData()));
 tableRf->setEnabled(false);
 write_btn->setEnabled(false);
}


/*!
 * @brief Создание меню.
 *
 * Создает виджеты меню в окне пользовательского интерфейса.
 */
void TTimerRf::create_Menu()
{
  QFont font = app_font;
  font.setBold(false);
  menuBar()->setFont(font);

  file_menu = menuBar()->addMenu(tr("&File"));
  exit = new QAction(tr("Exit"), this);
  file_menu->addAction(exit);
  connect(exit,SIGNAL(triggered(bool)), this, SLOT(close()));

}

/*!
 * @brief Создание статусной строки.
 *
 * Создает виджет статусной строки в окне пользовательского интерфейса.
 * В него входит информация об ошибке таймера, статусе таймера, где записываются результаты выполнения команд,
 * версии программного обеспечения.
 */
void TTimerRf::create_StatusBar()
{
  err_Label = new QLabel("Error: unknown");

  //err_Label->setFixedWidth(150);
  err_Label->setAlignment(Qt::AlignLeft);

  QLabel *version_label = new QLabel(this);
  version_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  version_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  version_label->setText("Program version: " + QCoreApplication::applicationVersion() + " ");

  status_Label=new QLabel(tr("Status: Start program"),this);
  status_Label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  hwver_Label=new QLabel(tr("HW version: unknown"),this);
  hwver_Label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusBar()->addWidget(err_Label,2);
  statusBar()->addWidget(status_Label,2);
  statusBar()->addWidget(hwver_Label,1);
  statusBar()->addWidget(version_label,1);

  QFont font = app_font;
  font.setBold(false);
  statusBar()->setFont(font);
}

/*!
 * @brief Обработчик нажатий клавиатуры.
 *
 * Отслеживает нажатия клавиатуры. Используется для определения модификации
 * входных данных.
 */
void TTimerRf::keyPressEvent(QKeyEvent *event)
{
//   qDebug()<<"keyPressed"<<event->key();
   modifyData=true;
   QWidget::keyPressEvent(event);
}

//-----------------------------PUBLIC SLOTS ----------------------------------------
/*!
 * @brief Слот приема сообщений и их визуализации.
 *
 * Создает виджет статусной строки в окне пользовательского интерфейса.
 * В него входит информация об ошибке таймера, статусе таймера, где записываются результаты выполнения команд,
 * версии программного обеспечения управляющей программы и таймера.
 * \param [in] msg - строка с сообщением.
 * \param [in] code - код вывода сообщения 0 - версия таймера,
 * 1 - статус таймера, 2 - ошибка, 3 - неустранимая ошибка (после возникновения
 * которой программа закроется в течение 10с), 4 - внешнее устройство отсутствует
 * (пользовательский интерфейс деактивируется), 5 - нешнее устройство присутствует
 * (пользовательский интерфейс активируется).
 */
void TTimerRf::slot_processMsg(QString msg, int code)
{
  if(code==5){ // device present
    tableRf->setEnabled(true);
    write_btn->setEnabled(true);
  }
  else if(code==4){ // device absent
    tableRf->setEnabled(false);
    write_btn->setEnabled(false);
  }
  else if(code==3) {
    status_Label->setText(msg);
    QTimer::singleShot(10000, qApp, SLOT(closeAllWindows()));
    QMessageBox::critical(this,"Error",msg);
      //QEventLoop loop;

      //loop.exec();
    //QMessageBox msgBox;

    //msgBox.setIcon(QMessageBox::Critical);
    //msgBox.setText(msg);
    //msgBox.exec();
    qApp->closeAllWindows();
  }
  else if(code==2){
    err_Label->setText("Error: "+msg);
  }
  else if(code==1){
    status_Label->setText("Status: "+msg);
  }
  else if(code==0) {
    hwver_Label->setText("HW version: "+msg);
  }
}
/*!
 * @brief Слот приема сообщений о готовности данных.
 *
 * Вызов данного слота сигнализирует о готовности данных в потоке таймера.
 * По вызову данного слота считываются данный, переводятся в мс и записываются
 * в таблицу.
 * \param [in] code - код полученных данных 0 - время срабатывания каналов.
 */
void TTimerRf::slot_processData(int code)
{
  if(code==0){
    for(int i=0;i<ALLVECTORS;i++){
      data[0][i]=dev->getTime(i)/1000.0;
    }
    putDataToTable();
  }
}
/*!
 * @brief Слот обработки сообщений.
 *
 * Создает виджет статусной строки в окне пользовательского интерфейса.
 * В него входит информация об ошибке таймера, статусе таймера, где записываются результаты выполнения команд,
 * версии программного обеспечения.
 */
void TTimerRf::slot_updateHW(void)
{
   dev->setState(UPDATE_STATE);
}

/*!
 * @brief Слот обработки сообщений.
 *
 * Создает виджет статусной строки в окне пользовательского интерфейса.
 * В него входит информация об ошибке таймера, статусе таймера, где записываются результаты выполнения команд,
 * версии программного обеспечения.
 */
void TTimerRf::slot_writeData(void)
{
  if(modifyData){
    slot_processMsg("",4);// disable btn
    getDataFromTable();
    putDataToTable();
    for(int i=0;i<ALLVECTORS;i++){
      dev->setTime(i,round(data[0][i]*1000));
    }
    dev->setState(WRITE_STATE);
    modifyData=false;
  }
}
