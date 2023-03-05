#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCryptographicHash>
#include <fstream>
#include <soapH.h>
#include <stdlib.h>
#include <ns1.nsmap>
#include <string>
#include <qdebug.h>
#include <QRegExp>
#include <QRegularExpressionValidator>
#include <QFileDialog>
#include <QDateTime>
#include <QLocale>
#include "tinyxml2.h"
#include "libiban.h"
int usermainid=-1;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->stackedWidget->setCurrentIndex(0);
	ui->stackedWidget->setAnimation(QEasingCurve::Type::OutQuart);
	ui->stackedWidget->setSpeed(650);
	connect(ui->prev,&QAbstractButton::clicked,[this]{
		if(ui->stackedWidget->slideInPrev()){
			ui->prev->setEnabled(false);
			ui->next->setEnabled(false);
		}
	});
	connect(ui->next,&QAbstractButton::clicked,[this]{
		if(ui->stackedWidget->slideInNext()){
			ui->prev->setEnabled(false);
			ui->next->setEnabled(false);
		}
	});
	connect(ui->stackedWidget,&SlidingStackedWidget::animationFinished,[this]{
		ui->prev->setEnabled(true);
		ui->next->setEnabled(true);
	});
	QTimer *t = new QTimer(this);
	t->setInterval(1000);
	connect(t, &QTimer::timeout, [&]() {
	   QString time1 = QTime::currentTime().toString();
	   ui->time->setText(time1);
	} );
	t->start();

	initialize();
}

MainWindow::~MainWindow()
{
	delete ui;
}

std::string MainWindow::gen_UUID() {
	std::string UUID8;
	QSqlQuery uuid_query;
	uuid_query.prepare("SELECT RIGHT(UUID_SHORT(), 8);");
	if (uuid_query.exec()) {
		if (uuid_query.size() > 0) {
			while (uuid_query.next()) {
				UUID8 = uuid_query.value(0).toString().toStdString();
			}
		}
		return UUID8;
	}
	else {
		QMessageBox::critical(NULL, "Error", uuid_query.lastError().text());
		return "";
	}
}

void MainWindow::initialize()
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("localhost") ;
	db.setPort(3306);
	db.setUserName("root");
	db.setPassword("");
	db.setDatabaseName("bankprojekt");

	if(!db.open())   {
		QMessageBox::critical(this, "Error", db.lastError().text());
		return;
	}
	// Validation and button settings
	QRegularExpression DigitsOnly("[0-9]*");
	QRegularExpression NoSpecialCharacters("^[^±!@£$%^&*_+§¡€#¢§¶•ªº«\\/<>?:;|=.,]{1,120}$");
	QRegularExpression IBAN28("[0-9]{28}");
	ui->RecipientAccountNumber->setValidator(new QRegularExpressionValidator(DigitsOnly, ui->RecipientAccountNumber));
	ui->Recipient->setValidator(new QRegularExpressionValidator(NoSpecialCharacters, ui->RecipientAccountNumber));
	ui->RecipientAccountNumber->setValidator(new QRegularExpressionValidator(IBAN28, ui->RecipientAccountNumber));
	//ui->Amount->setValidator(new QDoubleValidator(0.01, 10000.0, 2, ui->Amount));
	ui->label_fillin->hide();
	ui->label_acc->hide();
	ui->label_name->hide();
	ui->label_amt->hide();
}

void MainWindow::on_login_button_clicked()
{
	QString useremail = ui->Email_Input->text();
	QString userpass = ui->Password_input->text();
	if (userpass.isEmpty() || useremail.isEmpty()) {
		ui->login_info->setText("Please input proper credentials.");
	}
	else {
		QByteArray hash384 = QCryptographicHash::hash(userpass.toLocal8Bit(),QCryptographicHash::Sha384);
		qInfo() << hash384.toHex();
		QString userpass_hashed = hash384.toHex();
		QSqlQuery query;
		query.prepare("SELECT client.ID, client.Firstname, client.Lastname, client.PrvtId from client WHERE client.Email = :username AND client.Pass_SHA384 = :password");
		query.bindValue(":username", useremail);
		query.bindValue(":password", userpass_hashed);
		if (query.exec()) {
			if (query.size() > 0) {
				int &idref = usermainid;
				while (query.next())
					idref = query.value(0).toInt();
				loggedCustomer currentUser(usermainid);
				QString clientname = QString::fromStdString(currentUser.get_firstname())+" "+QString::fromStdString(currentUser.get_lastname());
				ui->login_info->setText("Welcome "+clientname);
				ui->transferFromList->addItem(QString::fromStdString(currentUser.get_code_iban()));
				ui->Email_Input->clear();
				ui->Password_input->clear();
				ui->lab_client_value->setText(clientname);
				ui->lab_balance_value->setText(QString::number(currentUser.get_balance()) + " PLN");
				ui->lab_number_value->setText(QString::fromStdString(currentUser.get_code_nbr()));
				QTimer::singleShot(1000, [=](){
					ui->next->click();
				});

						}
			else {

				ui->login_info->setText("Login failed. Invalid username or password.");
				ui->Email_Input->clear();
				ui->Password_input->clear();
			}
		}
	}

	ui->StreetAndNumber->hide();
	ui->CityAndPostal->hide();
	ui->label_6->hide();
	ui->label_7->hide();
}

void MainWindow::pain_xml_database() {
	QString PainfileName = QFileDialog::getSaveFileName(this,
														tr("Save Transfer"), "",
														tr("Extensible Markup Language (*.xml);;All Files (*)"));
	if (PainfileName.isEmpty())
		return;
	int transaction_index=1;
	loggedCustomer currentUser(usermainid);
	time_t currenttime = time(0);
	tm *ttime = localtime(&currenttime);
	struct soap *soap = soap_new();       // a new gSOAP runtime engine context, for memory management, IO, etc.
	soap_mode(soap, SOAP_XML_DEFAULTNS|SOAP_C_UTFSTRING|SOAP_XML_INDENT); // soap settings

	// XML Document Begin
	ns1__Document Document;
	ns1__CustomerCreditTransferInitiationV11 CstmrCdtTrfInitn;

	// XML Group Header
	ns1__GroupHeader95 GrpHdr;
	ns1__PartyIdentification135 InitgPty;
	ns1__Party38Choice PrvtId;
	ns1__PersonIdentification13 Othr_13;
	ns1__GenericPersonIdentification1 Id_1;
	std::string fullMsgId = "AUR" + std::to_string(1900+ttime->tm_year) + std::to_string(1 + ttime->tm_mon) + std::to_string(ttime->tm_mday) + "XX" + gen_UUID() +QString::number(transaction_index).rightJustified(2, '0').toStdString(); //pelna data;
	GrpHdr.MsgId=fullMsgId;
	GrpHdr.CreDtTm = currenttime;
	GrpHdr.NbOfTxs="1"; // 1 transakcja na plik - pojedyncze przelewy
	PrvtId.__union_Party38Choice = SOAP_UNION__ns1__union_Party38Choice_PrvtId;
	Id_1.Id=currentUser.get_prvtid();
	Othr_13.Othr.push_back(&Id_1);
	PrvtId.union_Party38Choice.PrvtId=&Othr_13;
	InitgPty.Id = &PrvtId;
	GrpHdr.InitgPty=&InitgPty;
	CstmrCdtTrfInitn.GrpHdr=&GrpHdr;

	//XML Payment Information
	ns1__PaymentInstruction40 PmtInf;
	ns1__DateAndDateTime2Choice DateTimeChoice;
	ns1__PartyIdentification135 Dbtr;
	ns1__PostalAddress24 PstlAdr;
	ns1__CashAccount40 DbtrAcct;
	ns1__AccountIdentification4Choice DbtrAcctId;
	ns1__BranchAndFinancialInstitutionIdentification6 DbtrAgt;
	ns1__FinancialInstitutionIdentification18 FinInstnId;
	ns1__ClearingSystemMemberIdentification2 ClrSysMmbId;
	ns1__ClearingSystemIdentification2Choice ClrSysId;
	std::string ClearingCode="PLKNR"; // stala wartosc
	std::string IBAN = currentUser.get_code_iban();
	PmtInf.PmtInfId=gen_UUID(); //  Identyfikator Bloku Płatności
	PmtInf.PmtMtd = ns1__PaymentMethod3Code__TRF;
	DateTimeChoice.__union_DateAndDateTime2Choice=SOAP_UNION__ns1__union_DateAndDateTime2Choice_Dt;

	std::string data=std::to_string(1900+ttime->tm_year) +"-" + std::to_string(1 + ttime->tm_mon) +"-"+std::to_string(ttime->tm_mday); //data
	DateTimeChoice.union_DateAndDateTime2Choice.Dt=&data;
	PmtInf.ReqdExctnDt = &DateTimeChoice;
	std::string namelastname = currentUser.get_firstname().append(" ").append(currentUser.get_lastname());
	std::string country = currentUser.get_country();
	Dbtr.Nm=&namelastname;
	PstlAdr.Ctry=&country;
	PstlAdr.AdrLine.push_back(currentUser.get_addressline1());
	PstlAdr.AdrLine.push_back(currentUser.get_addressline2());
	Dbtr.PstlAdr=&PstlAdr;
	PmtInf.Dbtr = &Dbtr;

	DbtrAcctId.__union_AccountIdentification4Choice=SOAP_UNION__ns1__union_AccountIdentification4Choice_IBAN;
	DbtrAcctId.union_AccountIdentification4Choice.IBAN=&IBAN;
	DbtrAcct.Id=&DbtrAcctId;
	PmtInf.DbtrAcct = &DbtrAcct;

	ClrSysId.__union_ClearingSystemIdentification2Choice=SOAP_UNION__ns1__union_ClearingSystemIdentification2Choice_Cd;
	ClrSysId.union_ClearingSystemIdentification2Choice.Cd=&ClearingCode;
	ClrSysMmbId.MmbId=currentUser.get_code_nbr().substr(0,8); // Identyfikator banku zleceniodawcy, przyjmujemy 8 pierwszych cyfr numeru konta
	ClrSysMmbId.ClrSysId=&ClrSysId;
	FinInstnId.ClrSysMmbId=&ClrSysMmbId;
	DbtrAgt.FinInstnId=&FinInstnId;
	PmtInf.DbtrAgt=&DbtrAgt;

	CstmrCdtTrfInitn.PmtInf.push_back(&PmtInf);

	ns1__CreditTransferTransaction54 CdtTrfTxInf;
	ns1__PaymentIdentification6 PmtId;
	ns1__Max35Text InstrId;
	ns1__Max35Text EndToEndId;
	ns1__PaymentTypeInformation26 PmtTpInf;
	ns1__ServiceLevel8Choice SvcLvl;
	ns1__AmountType4Choice Amt;
	ns1__ActiveOrHistoricCurrencyAndAmount Ccy;
	InstrId=gen_UUID(); //Identyfikator pojedynczej transakcji
	std::string prtry = "PLKR"; // stala wartosc
	PmtId.InstrId = &InstrId;
	if(ui->Reference->text().toStdString()!="") {
		PmtId.EndToEndId=ui->Reference->text().toStdString(); // referencja
	}
	else {
		PmtId.EndToEndId=" "; // brak referencji
	}
	SvcLvl.__union_ServiceLevel8Choice=SOAP_UNION__ns1__union_ServiceLevel8Choice_Cd;
	//if(ui->RTGSCheckBox->isChecked()) {
	std::string SvcLvlCd="RTGS"; //gdy pilne zlecenie typu sorbnet
	SvcLvl.union_ServiceLevel8Choice.Cd=&SvcLvlCd;
	PmtTpInf.SvcLvl.push_back(&SvcLvl);
	CdtTrfTxInf.PmtTpInf=&PmtTpInf;
	//}
	Amt.__union_AmountType4Choice=SOAP_UNION__ns1__union_AmountType4Choice_InstdAmt;
	Ccy.Ccy="PLN";
	Ccy.__item=QString::number(ui->Amount->value()).toStdString();
	Amt.union_AmountType4Choice.InstdAmt=&Ccy;
	CdtTrfTxInf.Amt=&Amt;
	ns1__BranchAndFinancialInstitutionIdentification6 CdtrAgt;
	ns1__FinancialInstitutionIdentification18 FinInstnId2;
	ns1__ClearingSystemMemberIdentification2 ClrSysMmbId2;
	ClrSysMmbId2.MmbId=ui->RecipientAccountNumber->text().toStdString().substr(0,8); // Identyfikator banku beneficjenta, przyjmujemy 8 pierwszych cyfr numeru konta
	FinInstnId2.ClrSysMmbId=&ClrSysMmbId2;
	CdtrAgt.FinInstnId=&FinInstnId2;
	CdtTrfTxInf.CdtrAgt=&CdtrAgt;
	std::string RecipientName = ui->Recipient->text().toStdString();
	ns1__PartyIdentification135 Cdtr;
	ns1__PostalAddress24 CdtrPstlAddr;
	ns1__CashAccount40 CdtrAcct;
	ns1__AccountIdentification4Choice CdtrAcctId;
	ns1__GenericAccountIdentification1 CdtrAcctOthr;
	ns1__Purpose2Choice Purp;
	ns1__RemittanceInformation21 RmtInf;
	Cdtr.Nm=&RecipientName;
	CdtrPstlAddr.Ctry=&country;
	if(ui->AddressCheckBox->isChecked()) {
		CdtrPstlAddr.AdrLine.push_back(ui->StreetAndNumber->text().toStdString());
		CdtrPstlAddr.AdrLine.push_back(ui->CityAndPostal->text().toStdString());
		Cdtr.PstlAdr=&CdtrPstlAddr;
	}
	CdtrAcctId.__union_AccountIdentification4Choice = SOAP_UNION__ns1__union_AccountIdentification4Choice_Othr;
	CdtrAcctOthr.Id = ui->RecipientAccountNumber->text().toStdString();
	CdtrAcctId.union_AccountIdentification4Choice.Othr = &CdtrAcctOthr;
	CdtrAcct.Id=&CdtrAcctId;
	Purp.__union_Purpose2Choice = SOAP_UNION__ns1__union_Purpose2Choice_Prtry;
	Purp.union_Purpose2Choice.Prtry=&prtry;
	RmtInf.Ustrd.push_back(ui->TransferTitle->text().toStdString());
	CdtTrfTxInf.PmtId=&PmtId;
	CdtTrfTxInf.Cdtr=&Cdtr;
	CdtTrfTxInf.CdtrAcct=&CdtrAcct;
	CdtTrfTxInf.Purp=&Purp;
	CdtTrfTxInf.RmtInf=&RmtInf;
	PmtInf.CdtTrfTxInf.push_back(&CdtTrfTxInf);
	Document.CstmrCdtTrfInitn = &CstmrCdtTrfInitn;
	std::ofstream out (PainfileName.toStdString());
	soap->os = &out;
	if (soap_write_ns1__Document(soap, &Document) != SOAP_OK) // write XML data to pain_przyklad
		soap_print_fault(soap, stderr);

	soap_destroy(soap);                   // delete managed C++ instances
	soap_end(soap);                       // delete other data
	soap_free(soap);                      // we're done
	using namespace tinyxml2;
	using namespace std;
	tinyxml2::XMLDocument pain_doc;
	if(pain_doc.LoadFile(PainfileName.toStdString().c_str())==0) { // 0 on succesful load
		// Some elements for later
		XMLElement * pRootElement = pain_doc.RootElement();
		XMLElement * pCstmrCdtTrfInitn = pRootElement -> FirstChildElement("CstmrCdtTrfInitn");
		XMLElement * pGrpHdr = pCstmrCdtTrfInitn -> FirstChildElement("GrpHdr");
		XMLElement * pPmtInf = pCstmrCdtTrfInitn -> FirstChildElement("PmtInf");
		XMLElement * pCdtTrf = pPmtInf ->FirstChildElement("CdtTrfTxInf");
		if (pRootElement != NULL || pCstmrCdtTrfInitn != NULL || pCstmrCdtTrfInitn != NULL || pCdtTrf != NULL) { // if any of the main trees is empty then the xml file is incompelete and cannot be parsed
			string pMsgId, pCreDtTm, pNbOfTxs, pInitgPty_ID, pPmtInfId, pPmtMtd, pReqdExctnDt_Dt, pDbtr_Nm, pDbtr_PstlAdr_Ctry, pDbtr_PstlAdr_AdrLine1, pDbtr_PstlAdr_AdrLine2, pDbtrAcct_IBAN, pDbtrAgtCd , pDbtrAgtMmb, pPmtIdInstr, pPmtIdEnd, pPmtTpCd, pAmt, pAmtCcy, pCdtrAgtMmbId, pCdtr_Nm, pCdtr_PstlAdr_Ctry, pCdtr_PstlAdr_AdrLine1 , pCdtr_PstlAdr_AdrLine2 , pCdtrAcct_Id, pPurp , pUstrd;
			// Group Header
			pMsgId = pGrpHdr->FirstChildElement("MsgId")->GetText();
			pCreDtTm = pGrpHdr->FirstChildElement("CreDtTm")->GetText();
			pNbOfTxs = pGrpHdr->FirstChildElement("NbOfTxs")->GetText();
			pInitgPty_ID = pGrpHdr->FirstChildElement("InitgPty")->FirstChildElement("Id")->FirstChildElement("PrvtId")->FirstChildElement("Othr")->FirstChildElement("Id")->GetText();

			// PmtInf
			pPmtInfId = pPmtInf->FirstChildElement("PmtInfId")->GetText();
			pPmtMtd = pPmtInf->FirstChildElement("PmtMtd")->GetText();
			pReqdExctnDt_Dt = pPmtInf->FirstChildElement("ReqdExctnDt")->FirstChildElement("Dt")->GetText();

			// Debtor information
			XMLElement * pDbtr = pPmtInf -> FirstChildElement("Dbtr");
			pDbtr_Nm = pDbtr->FirstChildElement("Nm")->GetText();
			if(pDbtr->FirstChildElement("PstlAdr")!= NULL) {
				pDbtr_PstlAdr_Ctry = pDbtr->FirstChildElement("PstlAdr")->FirstChildElement("Ctry")->GetText();
				pDbtr_PstlAdr_AdrLine1 = pDbtr->FirstChildElement("PstlAdr")->FirstChildElement("AdrLine")->GetText();
				pDbtr_PstlAdr_AdrLine2 = pDbtr->FirstChildElement("PstlAdr")->LastChildElement("AdrLine")->GetText();
			}
			else {
				pDbtr_PstlAdr_Ctry = "";
				pDbtr_PstlAdr_AdrLine1 = "";
				pDbtr_PstlAdr_AdrLine2 = "";
			}
			pDbtrAcct_IBAN = pPmtInf->FirstChildElement("DbtrAcct")->FirstChildElement("Id")->FirstChildElement("IBAN")->GetText();

			// Debtor Agt
			pDbtrAgtCd = pPmtInf->FirstChildElement("DbtrAgt")->FirstChildElement("FinInstnId")->FirstChildElement("ClrSysMmbId")->FirstChildElement("ClrSysId")->FirstChildElement("Cd")->GetText();;
			pDbtrAgtMmb = pPmtInf->FirstChildElement("DbtrAgt")->FirstChildElement("FinInstnId")->FirstChildElement("ClrSysMmbId")->FirstChildElement("MmbId")->GetText();;

			// Transfer Information
			pPmtIdInstr = pCdtTrf->FirstChildElement("PmtId")->FirstChildElement("InstrId")->GetText();
			if(pCdtTrf->FirstChildElement("PmtId")->FirstChildElement("EndToEndId")->GetText()!=nullptr) {
				pPmtIdEnd = pCdtTrf->FirstChildElement("PmtId")->FirstChildElement("EndToEndId")->GetText();
			}
			else {
				pPmtIdEnd = " ";
			}
			if(pCdtTrf->FirstChildElement("PmtTpInf")!=NULL) {
				pPmtTpCd = pCdtTrf->FirstChildElement("PmtTpInf")->FirstChildElement("SvcLvl")->FirstChildElement("Cd")->GetText();
			}
			else {
				pPmtTpCd = "";
			}
			pAmt = pCdtTrf->FirstChildElement("Amt")->FirstChildElement("InstdAmt")->GetText();
			pAmtCcy = pCdtTrf->FirstChildElement("Amt")->FirstChildElement("InstdAmt")->Attribute("Ccy");
			pCdtrAgtMmbId = pCdtTrf->FirstChildElement("CdtrAgt")->FirstChildElement("FinInstnId")->FirstChildElement("ClrSysMmbId")->FirstChildElement("MmbId")->GetText();

			// Creditor Information
			pCdtr_Nm = pCdtTrf->FirstChildElement("Cdtr")->FirstChildElement("Nm")->GetText();
			if(pCdtTrf->FirstChildElement("Cdtr")->FirstChildElement("PstlAdr")!=NULL) {
				pCdtr_PstlAdr_Ctry = pCdtTrf->FirstChildElement("Cdtr")->FirstChildElement("PstlAdr")->FirstChildElement("Ctry")->GetText();
				pCdtr_PstlAdr_AdrLine1 = pCdtTrf->FirstChildElement("Cdtr")->FirstChildElement("PstlAdr")->FirstChildElement("AdrLine")->GetText();
				pCdtr_PstlAdr_AdrLine2 = pCdtTrf->FirstChildElement("Cdtr")->FirstChildElement("PstlAdr")->LastChildElement("AdrLine")->GetText();
			}
			else {
				pCdtr_PstlAdr_Ctry = "";
				pCdtr_PstlAdr_AdrLine1 = "";
				pCdtr_PstlAdr_AdrLine2 = "";
			}
			pCdtrAcct_Id = pCdtTrf->FirstChildElement("CdtrAcct")->FirstChildElement("Id")->FirstChildElement("Othr")->FirstChildElement("Id")->GetText();

			// Finish
			pPurp = pCdtTrf->FirstChildElement("Purp")->FirstChildElement("Prtry")->GetText();
			if(pCdtTrf->FirstChildElement("RmtInf")->FirstChildElement("Ustrd")->GetText()!=nullptr) {
				pUstrd = pCdtTrf->FirstChildElement("RmtInf")->FirstChildElement("Ustrd")->GetText();
			}
			else {
				pUstrd = " ";
			}
			/*
			cout<<"MsgId: "<<pMsgId<<endl<<"CreDtTm: "<<pCreDtTm<<endl<<"NbOfTxs: "<<pNbOfTxs<<endl<<"InitgPty_ID: "<<pInitgPty_ID<<endl;
			cout<<"PmtInfId: "<<pPmtInfId<<endl<<"PmtInf: "<<pPmtInf<<endl<<"ReqdExctnDt_Dt: "<<pReqdExctnDt_Dt<<endl;
			cout<<"Dbtr_Nm: "<<pDbtr_Nm<<endl<<"Dbtr_PstlAdr_Ctry: "<<pDbtr_PstlAdr_Ctry<<endl<<"Dbtr_PstlAdr_AdrLine1: "<<pDbtr_PstlAdr_AdrLine1<<endl<<"Dbtr_PstlAdr_AdrLine2: "<<pDbtr_PstlAdr_AdrLine2<<endl<<"DbtrAcct_IBAN: "<<pDbtrAcct_IBAN<<endl;
			cout<<"Dbtr_AgtCd: "<<pDbtrAgtCd<<endl<<"Dbtr_AgtMmb: "<<pDbtrAgtMmb<<endl;
			cout<<"PmtIdInstr: "<<pPmtIdInstr<<endl<<"PmtIdEnd: "<<pPmtIdEnd<<endl<<"PmtTpCd: "<<pPmtTpCd<<endl<<"Amt: "<<pAmt<<endl<<"AmtCcy: "<<pAmtCcy<<"CdtrAgtMmbId: "<<pCdtrAgtMmbId<<endl;
			cout<<"Cdtr_Nm: "<<pCdtr_Nm<<endl<<"Cdtr_PstlAdr_Ctry: "<<pCdtr_PstlAdr_Ctry<<endl<<"Cdtr_PstlAdr_AdrLine1: "<<pCdtr_PstlAdr_AdrLine1<<endl<<"Cdtr_PstlAdr_AdrLine2: "<<pCdtr_PstlAdr_AdrLine2<<endl<<"CdtrAcct_Id: "<<pCdtrAcct_Id<<endl;
			cout<<"Purp: "<<pPurp<<endl<<"Ustrd: "<<pUstrd;
			*/
			QSqlQuery query_insert;
			query_insert.prepare("INSERT INTO transactions (Payment_File_MessageID, Time_Date, Number_Transactions, Client_PrivateID, Payment_BlockID, Operation_Type_TRF, Delivery_Date, Orginator_Name, Orginator_Country, Orginator_Address_Line1, Orginator_Address_Line2, Orginator_IBAN, Clearing_Code, Orginator_BankID, Payment_TransactionID, Reference, SORBNET, Amount, Currency, Recipient_BankID, Recipient_Name, Recipient_Country, Recipient_Address_Line1, Recipient_Address_Line2, Recipient_Account_Number, Operation_Type_PLKR, Transfer_Title) VALUES (:Payment_File_MessageID, :Time_Date, :Number_Transactions, :Client_PrivateID, :Payment_BlockID, :Operation_Type_TRF, :Delivery_Date, :Orginator_Name, :Orginator_Country, :Orginator_Address_Line1, :Orginator_Address_Line2, :Orginator_IBAN, :Clearing_Code, :Orginator_BankID, :Payment_TransactionID, :Reference, :SORBNET, :Amount, :Currency, :Recipient_BankID, :Recipient_Name, :Recipient_Country, :Recipient_Address_Line1, :Recipient_Address_Line2, :Recipient_Account_Number, :Operation_Type_PLKR, :Transfer_Title);");
			query_insert.bindValue(":Payment_File_MessageID", QString::fromStdString(pMsgId));
			//query_insert.bindValue(":Time_Date", QString::fromStdString(pCreDtTm));
			query_insert.bindValue(":Time_Date", QDateTime::fromString(QString::fromStdString(pCreDtTm),Qt::ISODate));
			query_insert.bindValue(":Number_Transactions", QString::fromStdString(pNbOfTxs));
			query_insert.bindValue(":Client_PrivateID", QString::fromStdString(pInitgPty_ID));
			query_insert.bindValue(":Payment_BlockID", QString::fromStdString(pPmtInfId));
			query_insert.bindValue(":Operation_Type_TRF", QString::fromStdString(pPmtMtd));
			//query_insert.bindValue(":Delivery_Date", QString::fromStdString(pReqdExctnDt_Dt));
			query_insert.bindValue(":Delivery_Date", QDate::fromString(QString::fromStdString(pReqdExctnDt_Dt),"yyyy-M-d"));
			query_insert.bindValue(":Orginator_Name", QString::fromStdString(pDbtr_Nm));
			query_insert.bindValue(":Orginator_Country", QString::fromStdString(pDbtr_PstlAdr_Ctry));
			query_insert.bindValue(":Orginator_Address_Line1", QString::fromStdString(pDbtr_PstlAdr_AdrLine1));
			query_insert.bindValue(":Orginator_Address_Line2", QString::fromStdString(pDbtr_PstlAdr_AdrLine2));
			query_insert.bindValue(":Orginator_IBAN", QString::fromStdString(pDbtrAcct_IBAN));
			query_insert.bindValue(":Clearing_Code", QString::fromStdString(pDbtrAgtCd));
			query_insert.bindValue(":Orginator_BankID", QString::fromStdString(pDbtrAgtMmb));
			query_insert.bindValue(":Payment_TransactionID", QString::fromStdString(pPmtIdInstr));
			query_insert.bindValue(":Reference", QString::fromStdString(pPmtIdEnd));
			query_insert.bindValue(":SORBNET", QString::fromStdString(pPmtTpCd));
			query_insert.bindValue(":Amount", QString::fromStdString(pAmt).toDouble());
			query_insert.bindValue(":Currency", QString::fromStdString(pAmtCcy));
			query_insert.bindValue(":Recipient_BankID", QString::fromStdString(pCdtrAgtMmbId));
			query_insert.bindValue(":Recipient_Name", QString::fromStdString(pCdtr_Nm));
			query_insert.bindValue(":Recipient_Country", QString::fromStdString(pCdtr_PstlAdr_Ctry));
			query_insert.bindValue(":Recipient_Address_Line1", QString::fromStdString(pCdtr_PstlAdr_AdrLine1));
			query_insert.bindValue(":Recipient_Address_Line2", QString::fromStdString(pCdtr_PstlAdr_AdrLine2));
			query_insert.bindValue(":Recipient_Account_Number", QString::fromStdString(pCdtrAcct_Id));
			query_insert.bindValue(":Operation_Type_PLKR", QString::fromStdString(pPurp));
			query_insert.bindValue(":Transfer_Title", QString::fromStdString(pUstrd));
			if(query_insert.exec()) {
				QMessageBox::information(NULL, "Query Success",  query_insert.lastQuery());
			}
			else QMessageBox::critical(NULL, "Error", query_insert.lastError().text());
		} else QMessageBox::critical(NULL, "Error", "Error opening file");
	}
	else {
		QMessageBox::critical(this, "Error", "XML File Error");
		return;
	}
	clear_fields();
}

void MainWindow::on_pushButton_clicked()

{
	boolean rec=false, amt=false, ibanv=false;
	if(ui->RecipientAccountNumber->text().length()!=26) {
		ui->RecipientAccountNumber->setFocus();
		ui->label_fillin->show();
		ui->label_acc->show();
	}
	else {
		QString iban_recipient = "PL"+ui->RecipientAccountNumber->text();
		auto iban = IBAN::IBAN::createFromString(iban_recipient.toStdString());
		if(!iban.validate()) {
			ui->RecipientAccountNumber->setFocus();
			ui->label_acc->show();
		}
		else {
			ibanv=true;
			ui->label_acc->hide();
		}
	}
	if(ui->Recipient->text().isEmpty()) {
		ui->Recipient->setFocus();
		ui->label_fillin->show();
		ui->label_name->show();
	}
	else {
		rec=true;
		ui->label_name->hide();
	}

	if(ui->Amount->text().isEmpty() || ui->Amount->value()<=0.0) {
		ui->Amount->setFocus();
		ui->label_fillin->show();
		ui->label_amt->show();
	}
	else {
		amt=true;
		ui->label_amt->hide();
	}
	if(rec && amt && ibanv) {
		ui->label_fillin->hide();
		pain_xml_database(); // initialize xml serialization -> deserialization -> to database process
	}
}

void MainWindow::clear_fields() {
	const QList<QLineEdit*> lineEdits = ui->tabWidget->findChildren<QLineEdit*>();
	for (QLineEdit *lineEdit : lineEdits) {
		lineEdit->clear();
	}
	ui->label_fillin->hide();
	ui->label_acc->hide();
}

void MainWindow::on_AddressCheckBox_stateChanged(int arg1)
{
	int amount_y=ui->Amount->y();
	int tt_y=ui->TransferTitle->y();
	int l4_y=ui->label_4->y();
	int l5_y=ui->label_5->y();
	int l8_y=ui->label_8->y();
	if(arg1) {ui->label_8->y();
		ui->Amount->move(ui->Amount->x(),amount_y+120);
		ui->TransferTitle->move(ui->TransferTitle->x(),tt_y+120);
		ui->label_4->move(ui->label_4->x(),l4_y+120);
		ui->label_5->move(ui->label_5->x(),l5_y+120);
		ui->label_amt->move(ui->label_amt->x(),l5_y+120);
		ui->label_8->move(ui->label_8->x(),l8_y+120);
		ui->StreetAndNumber->show();
		ui->CityAndPostal->show();
		ui->label_6->show();
		ui->label_7->show();
	}
	else {
		ui->StreetAndNumber->clear();
		ui->CityAndPostal->clear();
		ui->Amount->move(ui->Amount->x(),amount_y-120);
		ui->label_4->move(ui->label_4->x(),l4_y-120);
		ui->label_5->move(ui->label_5->x(),l5_y-120);
		ui->label_amt->move(ui->label_amt->x(),l5_y-120);
		ui->TransferTitle->move(ui->TransferTitle->x(),tt_y-120);
		ui->label_8->move(ui->label_8->x(),l8_y-120);
		ui->StreetAndNumber->hide();
		ui->CityAndPostal->hide();
		ui->label_6->hide();
		ui->label_7->hide();
	}
}
