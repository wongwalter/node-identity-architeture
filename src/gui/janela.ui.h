/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void janela::salvar()
{
    nidcore = 'dead::dead'
    nidrvs = 'dead::dead'
    iprvs = '0.0.0.0'
    timervs = '5'
    nidgw = 'dead::dead'
    ipgw = '0.0.0.0'
    ipdht = '0.0.0.0'
    portdht = '0'
    niddns = 'dead::dead'
    dns = 1
    dhcp = 1
    dht = 0
    primaryIface = "eth0"
    type = self.comboBox1.currentItem()
    nid = self.lineEdit1.text().ascii()
    nidcorehome = self.lineEdit2.text().ascii()
    if self.lineEdit3.text().ascii() != '':
        nidcore = self.lineEdit3.text().ascii()
    if self.lineEdit4.text().ascii() != '':
        nidrvs = self.lineEdit4.text().ascii()
    if self.lineEdit5.text().ascii() != '':
        iprvs = self.lineEdit5.text().ascii()
    if self.lineEdit6.text().ascii() != '':
        timervs = self.lineEdit6.text().ascii()
    if self.lineEdit7.text().ascii() != '':
        nidgw = self.lineEdit7.text().ascii()
    if self.lineEdit8.text().ascii() != '':
        ipgw = self.lineEdit8.text().ascii()
    if self.lineEdit9.text().ascii() != '':
        ipdht = self.lineEdit9.text().ascii()
    if self.lineEdit10.text().ascii() != '':
        portdht = self.lineEdit10.text().ascii()
    if self.lineEdit11.text().ascii() != '':
        niddns = self.lineEdit11.text().ascii()
    if self.lineEdit12.text().ascii() != '':
        primaryIface = self.lineEdit12.text().ascii()
    dns = self.checkBox1.isChecked()
    dhcp = self.checkBox2.isChecked()
    dht = self.checkBox3.isChecked()
    f = open('nid.conf', 'w')	
    f.write('<configuration>\n')
    f.write('<type>' + str(type) + '</type>\n')
    f.write('<nodeID>' + nid + '</nodeID>\n')
    f.write('<nidcorehome>' + nidcorehome + '</nidcorehome>\n')
    f.write('<nidcore>' + nidcore + '</nidcore>\n')
    f.write('<nidrvs>' + nidrvs + '</nidrvs>\n')
    f.write('<rvsIP>' + iprvs + '</rvsIP>\n')
    f.write('<rvsTimeout>' + timervs + '</rvsTimeout>\n')
    f.write('<nidgw>' + nidgw + '</nidgw>\n')
    f.write('<ip>' + ipgw + '</ip>\n')
    f.write('<dhtIP>' + ipdht + '</dhtIP>\n')
    f.write('<dhtPort>' + portdht + '</dhtPort>\n')
    f.write('<niddns>' + niddns + '</niddns>\n')
    f.write('<dns>' + str(int(dns)) + '</dns>\n')
    f.write('<dhcp>' + str(int(dhcp)) + '</dhcp>\n')
    f.write('<dht>' + str(int(dht))+ '</dht>\n')
    f.write('<primaryIface>' + primaryIface + '</primaryIface>\n')
    f.write('</configuration>\n')
    f.close()	    
}


void janela::nodeType()
{
     tipo = self.comboBox1.currentItem()
     if tipo == 0:
        self.checkBox2.setEnabled(True)
	self.checkBox2.setChecked(False)
        self.checkBox3.setEnabled(False)
        self.checkBox3.setChecked(False)
        self.lineEdit1.setEnabled(True)
        self.lineEdit2.setEnabled(True)
        self.lineEdit3.setEnabled(True)
        self.lineEdit4.setEnabled(True)
        self.lineEdit5.setEnabled(True)
        self.lineEdit6.setEnabled(True)
        self.lineEdit7.setEnabled(True)
        self.lineEdit8.setEnabled(True)
        self.lineEdit9.setEnabled(False)
        self.lineEdit10.setEnabled(False)
        self.lineEdit11.setEnabled(True)
     elif tipo == 1:
        self.checkBox2.setEnabled(False)
        self.checkBox2.setChecked(False)
        self.checkBox3.setEnabled(False)
        self.checkBox3.setChecked(False)
        self.lineEdit1.setEnabled(True)
        self.lineEdit2.setEnabled(True)
        self.lineEdit3.setEnabled(True)
        self.lineEdit4.setEnabled(False)
        self.lineEdit5.setEnabled(False)
        self.lineEdit6.setEnabled(True)
        self.lineEdit7.setEnabled(True)
        self.lineEdit8.setEnabled(True)
        self.lineEdit9.setEnabled(False)
        self.lineEdit10.setEnabled(False)
        self.lineEdit11.setEnabled(True)        
     elif tipo == 2:
        self.checkBox2.setEnabled(False)
        self.checkBox2.setChecked(False)
        self.checkBox3.setEnabled(False)
        self.checkBox3.setChecked(False)
        self.lineEdit1.setEnabled(True)
        self.lineEdit2.setEnabled(True)
        self.lineEdit3.setEnabled(True)
        self.lineEdit4.setEnabled(True)
        self.lineEdit5.setEnabled(True)
        self.lineEdit6.setEnabled(True)
        self.lineEdit7.setEnabled(True)
        self.lineEdit8.setEnabled(True)
        self.lineEdit9.setEnabled(False)
        self.lineEdit10.setEnabled(False)
        self.lineEdit11.setEnabled(True)        
     elif tipo == 3:
        self.checkBox2.setEnabled(False)
        self.checkBox2.setChecked(False)
        self.checkBox3.setEnabled(True)
        self.checkBox3.setChecked(True)
        self.lineEdit1.setEnabled(True)
        self.lineEdit2.setEnabled(True)
        self.lineEdit3.setEnabled(True)
        self.lineEdit4.setEnabled(True)
        self.lineEdit5.setEnabled(True)
        self.lineEdit6.setEnabled(True)
        self.lineEdit7.setEnabled(False)
        self.lineEdit8.setEnabled(False)
        self.lineEdit9.setEnabled(True)
        self.lineEdit10.setEnabled(True)
        self.lineEdit11.setEnabled(True)
}
