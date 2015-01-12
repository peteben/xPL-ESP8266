using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

// Crappy little program to test the ESP8266 sending and receiving xPL messages.
// Normally, proper xPL programs make use of a 'hub' server, which repeats xPL messages to all
// registered applications, but for simplicity, this app listens to every packet sent to port 3865.
// Crude parsing is then used to display command packets that might be coming from the ESP8266

// Do not use this as a basis to write any sort of proper xPL-compatible service or device!
// Instead, use the official xPL libraries that can be found at http://xplproject.co.uk

namespace xPL_tester {
	public partial class Form1 : Form {
		const string VENDORID = "peteben";
		const string DEVICEID = "xpltester";
		UdpClient cl;
		IPEndPoint ep;

		delegate void StringDelegate(string data);

		public void ReceiveCallback(IAsyncResult ar) {
			Byte[] receiveBytes = cl.EndReceive(ar, ref ep);
			string receiveString = Encoding.ASCII.GetString(receiveBytes);
			string st, scmd, sdevice;

			// Crude parser looks for x10 command packets comming from the ESP8266
			if (receiveString.Contains("ESP8266") && receiveString.Contains("x10.basic")) {
				int pos = receiveString.IndexOf("command=");

				if (pos >= 0) {
					st = receiveString.Substring(pos);
					int k = st.IndexOf('\n');
					if (k >= 8) {
						scmd = st.Substring(8, k - 8);
						pos = receiveString.IndexOf("device=");
						if (pos >= 0) {
							st = receiveString.Substring(pos);
							k = st.IndexOf('\n');
							if (k >= 7) {
								sdevice = st.Substring(7, k - 7);
								Invoke(new StringDelegate(AddText), new[] { sdevice + " " + scmd });
								}
							}
						}


					}
				}
			cl.BeginReceive(new AsyncCallback(ReceiveCallback), null);
			}

		public Form1() {
			InitializeComponent();
			cl = new UdpClient(3865);
			ep = new IPEndPoint(IPAddress.Any, 3865);
			cl.EnableBroadcast = true;

			cl.BeginReceive(new AsyncCallback(ReceiveCallback), null);
			}

		void AddText(string s) {
			textBox1.Text = s + "\r\n" + textBox1.Text;
			textBox1.Refresh();
			}

		// Send a UDP xPL message to the ESP8266, to turn the LED (GPIO0) on.
		private void OnBut_Click(object sender, EventArgs e) {
			string onMsg = "xpl-cmnd\n{\nhop=1\nsource=peteben-xpltester.ESP\ntarget=*\n}\nx10.basic\n{\ncommand=on\ndevice=" + textBox2.Text +"\n}\n";
			Byte[] sendBytes = Encoding.ASCII.GetBytes(onMsg);
			try{
				cl.Send(sendBytes, sendBytes.Length,new IPEndPoint(IPAddress.Broadcast, 3865));
				}
			catch ( Exception ex ){
				}
			}

		// Send a UDP xPL message to the ESP8266, to turn the LED (GPIO0) off.
		private void OffBut_Click(object sender, EventArgs e) {
			string offMsg = "xpl-cmnd\n{\nhop=1\nsource=peteben-xpltester.ESP\ntarget=*\n}\nx10.basic\n{\ncommand=off\ndevice=" + textBox2.Text + "\n}\n";
			Byte[] sendBytes = Encoding.ASCII.GetBytes(offMsg);
			try {
				cl.Send(sendBytes, sendBytes.Length, new IPEndPoint(IPAddress.Broadcast, 3865));
				}
			catch (Exception ex) {
				}
			}

		private void QuitBut_Click(object sender, EventArgs e) {
			Close();
			}
		}
	}
