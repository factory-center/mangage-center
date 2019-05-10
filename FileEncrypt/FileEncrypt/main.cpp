#include "FileEncrypt.h"
#include <QtWidgets/QApplication>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FileEncrypt w;
	w.show();
	return a.exec();
}
