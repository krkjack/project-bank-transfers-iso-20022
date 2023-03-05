#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QMessageBox>
#include <iostream>
#include <QTextCodec>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSqlTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_login_button_clicked();
    void initialize();
	void clear_fields();
    std::string gen_UUID();
	void pain_xml_database();
    void on_pushButton_clicked();
    void on_AddressCheckBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *mModel;
    QSqlDatabase db;
};

class loggedCustomer {
  private:
    float balance;
    std::string firstname;
    std::string lastname;
    std::string prvtid;
    std::string addressline1;
    std::string addressline2;
    std::string country;
    std::string code_nbr;
    std::string code_iban;
  public:
    loggedCustomer(int mainid) {
        QSqlQuery query;
        query.prepare("SELECT client.firstname, client.lastname, client.prvtid, client.street, client.nr_property, client.nr_apartament, client.zip_code, client.town, client.country, account.account_number, account.IBAN, account.balance, account.interest FROM client JOIN account on client.ID = account.ID_client WHERE client.id = :mainid");
        query.bindValue(":mainid", mainid);
        if (query.exec()) {
            if (query.size() > 0) {
                while (query.next()) {
                    firstname = query.value(0).toString().toStdString();
                    lastname = query.value(1).toString().toStdString();
                    prvtid = query.value(2).toString().toStdString();
                    if(query.value(5).toString().toStdString()=="")
                        addressline1 = query.value(3).toString().toStdString().append(" ").append(query.value(4).toString().toStdString());
                    else
                        addressline1 = query.value(3).toString().toStdString().append(" ").append(query.value(4).toString().toStdString().append("/").append(query.value(5).toString().toStdString()));
                    addressline2 = query.value(6).toString().toStdString().append(" ").append(query.value(7).toString().toStdString());
                    country = query.value(8).toString().toStdString();
                    code_nbr = query.value(9).toString().toStdString();
                    code_iban = query.value(10).toString().toStdString();
                    balance = query.value(11).toString().toFloat();

                }
            }
						   //std::cout<<mainid<<std::endl<<firstname<<std::endl<<lastname<<std::endl<<prvtid<<std::endl<<addressline1<<std::endl<<addressline2<<std::endl<<code_iban<<std::endl<<code_nbr;
        }
        else {
            QMessageBox::critical(NULL, "Error", query.lastError().text());
            return;
        }
    }
    std::string get_firstname() {
        return firstname;
    }
    std::string get_lastname() {
        return lastname;;
    }
     std::string get_prvtid() {
        return prvtid;
    }
    std::string get_addressline1() {
        return addressline1;
    }
    std::string get_addressline2() {
        return addressline2;
    }
    std::string get_code_nbr() {
        return code_nbr;
    }
    std::string get_code_iban() {
        return code_iban;
    }
    std::string get_country() {
        return country;
    }
	float get_balance() {
		return balance;
	}
};
#endif // MAINWINDOW_H
