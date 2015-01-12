namespace xPL_tester {
	partial class Form1 {
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing) {
			if (disposing && (components != null)) {
				components.Dispose();
				}
			base.Dispose(disposing);
			}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent() {
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.OnBut = new System.Windows.Forms.Button();
			this.OffBut = new System.Windows.Forms.Button();
			this.QuitBut = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// textBox1
			// 
			this.textBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.textBox1.Location = new System.Drawing.Point(13, 33);
			this.textBox1.Multiline = true;
			this.textBox1.Name = "textBox1";
			this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.textBox1.Size = new System.Drawing.Size(488, 184);
			this.textBox1.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(13, 14);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(104, 13);
			this.label1.TabIndex = 1;
			this.label1.Text = "Messages Received";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(13, 236);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(41, 13);
			this.label2.TabIndex = 2;
			this.label2.Text = "Device";
			// 
			// textBox2
			// 
			this.textBox2.Location = new System.Drawing.Point(59, 233);
			this.textBox2.Name = "textBox2";
			this.textBox2.Size = new System.Drawing.Size(36, 20);
			this.textBox2.TabIndex = 3;
			this.textBox2.Text = "C1";
			// 
			// OnBut
			// 
			this.OnBut.Location = new System.Drawing.Point(121, 231);
			this.OnBut.Name = "OnBut";
			this.OnBut.Size = new System.Drawing.Size(75, 23);
			this.OnBut.TabIndex = 4;
			this.OnBut.Text = "ON";
			this.OnBut.UseVisualStyleBackColor = true;
			this.OnBut.Click += new System.EventHandler(this.OnBut_Click);
			// 
			// OffBut
			// 
			this.OffBut.Location = new System.Drawing.Point(202, 231);
			this.OffBut.Name = "OffBut";
			this.OffBut.Size = new System.Drawing.Size(75, 23);
			this.OffBut.TabIndex = 5;
			this.OffBut.Text = "OFF";
			this.OffBut.UseVisualStyleBackColor = true;
			this.OffBut.Click += new System.EventHandler(this.OffBut_Click);
			// 
			// QuitBut
			// 
			this.QuitBut.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.QuitBut.Location = new System.Drawing.Point(426, 231);
			this.QuitBut.Name = "QuitBut";
			this.QuitBut.Size = new System.Drawing.Size(75, 23);
			this.QuitBut.TabIndex = 6;
			this.QuitBut.Text = "Quit";
			this.QuitBut.UseVisualStyleBackColor = true;
			this.QuitBut.Click += new System.EventHandler(this.QuitBut_Click);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(513, 269);
			this.Controls.Add(this.QuitBut);
			this.Controls.Add(this.OffBut);
			this.Controls.Add(this.OnBut);
			this.Controls.Add(this.textBox2);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.textBox1);
			this.Name = "Form1";
			this.Text = "xPL Tester";
			this.ResumeLayout(false);
			this.PerformLayout();

			}

		#endregion

		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox textBox2;
		private System.Windows.Forms.Button OnBut;
		private System.Windows.Forms.Button OffBut;
		private System.Windows.Forms.Button QuitBut;
		}
	}

