#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
		Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	private slots:

		void on_PORTS_info_button_clicked();

		void on_COM_port_select_currentIndexChanged(const QString &arg1);

		void on_DATA_bits_select_currentIndexChanged(int index);

		void on_STOP_bits_select_currentIndexChanged(int index);

		void on_PARITY_select_currentIndexChanged(int index);

		void on_BAUD_edit_textChanged(const QString &arg1);

		void on_SEND_data_button_B_clicked();

		void on_SEND_data_button_C_clicked();

		void on_SEND_data_button_D_clicked();

		void on_SEND_data_button_A_clicked();

		void on_Send_frq_Button_clicked();

	private:
		Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
