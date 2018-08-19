namespace cs_napalm
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.tableLayoutPanel3 = new System.Windows.Forms.TableLayoutPanel();
            this.PlaylistView = new System.Windows.Forms.ListView();
            this.PlaylistIndex = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.TrackNo = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Artist = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Album = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.TrackName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Duration = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.ArtCoverLabel = new System.Windows.Forms.Label();
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.tableLayoutPanel4 = new System.Windows.Forms.TableLayoutPanel();
            this.tableLayoutPanel5 = new System.Windows.Forms.TableLayoutPanel();
            this.PreviousTrackButton = new System.Windows.Forms.Button();
            this.LoadButton = new System.Windows.Forms.Button();
            this.PlayButton = new System.Windows.Forms.Button();
            this.NextTrackButton = new System.Windows.Forms.Button();
            this.PauseButton = new System.Windows.Forms.Button();
            this.StopButton = new System.Windows.Forms.Button();
            this.SeekBar = new System.Windows.Forms.TrackBar();
            this.tableLayoutPanel6 = new System.Windows.Forms.TableLayoutPanel();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.TimeLabel = new System.Windows.Forms.Label();
            this.DurationLabel = new System.Windows.Forms.Label();
            this.TrackTitleLabel = new System.Windows.Forms.Label();
            this.TrackArtistLabel = new System.Windows.Forms.Label();
            this.AlbumLabel = new System.Windows.Forms.Label();
            this.tableLayoutPanel3.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.tableLayoutPanel2.SuspendLayout();
            this.tableLayoutPanel4.SuspendLayout();
            this.tableLayoutPanel5.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.SeekBar)).BeginInit();
            this.tableLayoutPanel6.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            this.statusStrip1.Location = new System.Drawing.Point(0, 520);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(771, 22);
            this.statusStrip1.TabIndex = 9;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // tableLayoutPanel3
            // 
            this.tableLayoutPanel3.ColumnCount = 1;
            this.tableLayoutPanel3.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel3.Controls.Add(this.PlaylistView, 0, 1);
            this.tableLayoutPanel3.Controls.Add(this.tableLayoutPanel1, 0, 0);
            this.tableLayoutPanel3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel3.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel3.Name = "tableLayoutPanel3";
            this.tableLayoutPanel3.RowCount = 2;
            this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel3.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel3.Size = new System.Drawing.Size(771, 520);
            this.tableLayoutPanel3.TabIndex = 10;
            // 
            // PlaylistView
            // 
            this.PlaylistView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.PlaylistIndex,
            this.TrackNo,
            this.Artist,
            this.Album,
            this.TrackName,
            this.Duration});
            this.PlaylistView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PlaylistView.Location = new System.Drawing.Point(3, 265);
            this.PlaylistView.Name = "PlaylistView";
            this.PlaylistView.Size = new System.Drawing.Size(765, 252);
            this.PlaylistView.TabIndex = 9;
            this.PlaylistView.UseCompatibleStateImageBehavior = false;
            this.PlaylistView.View = System.Windows.Forms.View.Details;
            // 
            // PlaylistIndex
            // 
            this.PlaylistIndex.Text = "Index";
            this.PlaylistIndex.Width = 34;
            // 
            // TrackNo
            // 
            this.TrackNo.Text = "Track No.";
            this.TrackNo.Width = 46;
            // 
            // Artist
            // 
            this.Artist.Text = "Artist";
            this.Artist.Width = 89;
            // 
            // Album
            // 
            this.Album.Text = "Album";
            this.Album.Width = 95;
            // 
            // TrackName
            // 
            this.TrackName.Text = "Title";
            this.TrackName.Width = 422;
            // 
            // Duration
            // 
            this.Duration.Text = "Duration";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.AutoSize = true;
            this.tableLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.ArtCoverLabel, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.tableLayoutPanel2, 1, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(765, 256);
            this.tableLayoutPanel1.TabIndex = 10;
            // 
            // ArtCoverLabel
            // 
            this.ArtCoverLabel.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.ArtCoverLabel.BackColor = System.Drawing.Color.Black;
            this.ArtCoverLabel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ArtCoverLabel.Image = global::cs_napalm.Properties.Resources.napalm_icon;
            this.ArtCoverLabel.Location = new System.Drawing.Point(0, 0);
            this.ArtCoverLabel.Margin = new System.Windows.Forms.Padding(0);
            this.ArtCoverLabel.MaximumSize = new System.Drawing.Size(256, 256);
            this.ArtCoverLabel.MinimumSize = new System.Drawing.Size(256, 256);
            this.ArtCoverLabel.Name = "ArtCoverLabel";
            this.ArtCoverLabel.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.ArtCoverLabel.Size = new System.Drawing.Size(256, 256);
            this.ArtCoverLabel.TabIndex = 10;
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.BackColor = System.Drawing.SystemColors.Control;
            this.tableLayoutPanel2.ColumnCount = 1;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel4, 0, 1);
            this.tableLayoutPanel2.Controls.Add(this.tableLayoutPanel6, 0, 0);
            this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel2.Location = new System.Drawing.Point(259, 3);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.RowCount = 2;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel2.Size = new System.Drawing.Size(503, 250);
            this.tableLayoutPanel2.TabIndex = 11;
            // 
            // tableLayoutPanel4
            // 
            this.tableLayoutPanel4.AutoSize = true;
            this.tableLayoutPanel4.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanel4.ColumnCount = 2;
            this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel4.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel4.Controls.Add(this.tableLayoutPanel5, 0, 0);
            this.tableLayoutPanel4.Controls.Add(this.SeekBar, 1, 0);
            this.tableLayoutPanel4.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel4.Location = new System.Drawing.Point(3, 196);
            this.tableLayoutPanel4.Name = "tableLayoutPanel4";
            this.tableLayoutPanel4.RowCount = 1;
            this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel4.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 51F));
            this.tableLayoutPanel4.Size = new System.Drawing.Size(497, 51);
            this.tableLayoutPanel4.TabIndex = 10;
            // 
            // tableLayoutPanel5
            // 
            this.tableLayoutPanel5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel5.AutoSize = true;
            this.tableLayoutPanel5.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.tableLayoutPanel5.ColumnCount = 6;
            this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel5.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel5.Controls.Add(this.PreviousTrackButton, 0, 0);
            this.tableLayoutPanel5.Controls.Add(this.LoadButton, 5, 0);
            this.tableLayoutPanel5.Controls.Add(this.PlayButton, 1, 0);
            this.tableLayoutPanel5.Controls.Add(this.NextTrackButton, 4, 0);
            this.tableLayoutPanel5.Controls.Add(this.PauseButton, 2, 0);
            this.tableLayoutPanel5.Controls.Add(this.StopButton, 3, 0);
            this.tableLayoutPanel5.Location = new System.Drawing.Point(3, 6);
            this.tableLayoutPanel5.Name = "tableLayoutPanel5";
            this.tableLayoutPanel5.RowCount = 1;
            this.tableLayoutPanel5.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel5.Size = new System.Drawing.Size(234, 39);
            this.tableLayoutPanel5.TabIndex = 7;
            // 
            // PreviousTrackButton
            // 
            this.PreviousTrackButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.PreviousTrackButton.BackgroundImage = global::cs_napalm.Properties.Resources.button_previous;
            this.PreviousTrackButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.PreviousTrackButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PreviousTrackButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.PreviousTrackButton.Location = new System.Drawing.Point(1, 1);
            this.PreviousTrackButton.Margin = new System.Windows.Forms.Padding(1);
            this.PreviousTrackButton.Name = "PreviousTrackButton";
            this.PreviousTrackButton.Size = new System.Drawing.Size(37, 37);
            this.PreviousTrackButton.TabIndex = 0;
            this.PreviousTrackButton.UseVisualStyleBackColor = true;
            this.PreviousTrackButton.Click += new System.EventHandler(this.PreviousTrackButton_Click);
            // 
            // LoadButton
            // 
            this.LoadButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.LoadButton.BackgroundImage = global::cs_napalm.Properties.Resources.button_load;
            this.LoadButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.LoadButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.LoadButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.LoadButton.Location = new System.Drawing.Point(196, 1);
            this.LoadButton.Margin = new System.Windows.Forms.Padding(1);
            this.LoadButton.Name = "LoadButton";
            this.LoadButton.Size = new System.Drawing.Size(37, 37);
            this.LoadButton.TabIndex = 5;
            this.LoadButton.UseVisualStyleBackColor = true;
            this.LoadButton.Click += new System.EventHandler(this.LoadButton_Click);
            // 
            // PlayButton
            // 
            this.PlayButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.PlayButton.BackgroundImage = global::cs_napalm.Properties.Resources.button_play;
            this.PlayButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.PlayButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PlayButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.PlayButton.Location = new System.Drawing.Point(40, 1);
            this.PlayButton.Margin = new System.Windows.Forms.Padding(1);
            this.PlayButton.Name = "PlayButton";
            this.PlayButton.Size = new System.Drawing.Size(37, 37);
            this.PlayButton.TabIndex = 1;
            this.PlayButton.UseVisualStyleBackColor = true;
            this.PlayButton.Click += new System.EventHandler(this.PlayButton_Click);
            // 
            // NextTrackButton
            // 
            this.NextTrackButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.NextTrackButton.BackgroundImage = global::cs_napalm.Properties.Resources.button_next;
            this.NextTrackButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.NextTrackButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.NextTrackButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NextTrackButton.Location = new System.Drawing.Point(157, 1);
            this.NextTrackButton.Margin = new System.Windows.Forms.Padding(1);
            this.NextTrackButton.Name = "NextTrackButton";
            this.NextTrackButton.Size = new System.Drawing.Size(37, 37);
            this.NextTrackButton.TabIndex = 4;
            this.NextTrackButton.UseVisualStyleBackColor = true;
            this.NextTrackButton.Click += new System.EventHandler(this.NextTrackButton_Click);
            // 
            // PauseButton
            // 
            this.PauseButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.PauseButton.BackgroundImage = global::cs_napalm.Properties.Resources.button_pause;
            this.PauseButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.PauseButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PauseButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.PauseButton.Location = new System.Drawing.Point(79, 1);
            this.PauseButton.Margin = new System.Windows.Forms.Padding(1);
            this.PauseButton.Name = "PauseButton";
            this.PauseButton.Size = new System.Drawing.Size(37, 37);
            this.PauseButton.TabIndex = 2;
            this.PauseButton.UseVisualStyleBackColor = true;
            this.PauseButton.Click += new System.EventHandler(this.PauseButton_Click);
            // 
            // StopButton
            // 
            this.StopButton.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.StopButton.BackgroundImage = global::cs_napalm.Properties.Resources.button_stop;
            this.StopButton.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.StopButton.Dock = System.Windows.Forms.DockStyle.Fill;
            this.StopButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.StopButton.Location = new System.Drawing.Point(118, 1);
            this.StopButton.Margin = new System.Windows.Forms.Padding(1);
            this.StopButton.Name = "StopButton";
            this.StopButton.Size = new System.Drawing.Size(37, 37);
            this.StopButton.TabIndex = 3;
            this.StopButton.UseVisualStyleBackColor = true;
            this.StopButton.Click += new System.EventHandler(this.StopButton_Click);
            // 
            // SeekBar
            // 
            this.SeekBar.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.SeekBar.Location = new System.Drawing.Point(243, 3);
            this.SeekBar.Maximum = 1;
            this.SeekBar.Name = "SeekBar";
            this.SeekBar.Size = new System.Drawing.Size(251, 45);
            this.SeekBar.TabIndex = 8;
            this.SeekBar.TickStyle = System.Windows.Forms.TickStyle.Both;
            // 
            // tableLayoutPanel6
            // 
            this.tableLayoutPanel6.ColumnCount = 2;
            this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel6.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel6.Controls.Add(this.label1, 0, 0);
            this.tableLayoutPanel6.Controls.Add(this.label2, 0, 1);
            this.tableLayoutPanel6.Controls.Add(this.label3, 0, 2);
            this.tableLayoutPanel6.Controls.Add(this.label4, 0, 3);
            this.tableLayoutPanel6.Controls.Add(this.label5, 0, 4);
            this.tableLayoutPanel6.Controls.Add(this.TimeLabel, 1, 3);
            this.tableLayoutPanel6.Controls.Add(this.DurationLabel, 1, 4);
            this.tableLayoutPanel6.Controls.Add(this.TrackTitleLabel, 1, 0);
            this.tableLayoutPanel6.Controls.Add(this.TrackArtistLabel, 1, 1);
            this.tableLayoutPanel6.Controls.Add(this.AlbumLabel, 1, 2);
            this.tableLayoutPanel6.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel6.Location = new System.Drawing.Point(3, 3);
            this.tableLayoutPanel6.Name = "tableLayoutPanel6";
            this.tableLayoutPanel6.RowCount = 6;
            this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel6.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel6.Size = new System.Drawing.Size(497, 187);
            this.tableLayoutPanel6.TabIndex = 11;
            // 
            // label1
            // 
            this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 3);
            this.label1.Margin = new System.Windows.Forms.Padding(3);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(30, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Title:";
            // 
            // label2
            // 
            this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(3, 22);
            this.label2.Margin = new System.Windows.Forms.Padding(3);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(33, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Artist:";
            // 
            // label3
            // 
            this.label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 41);
            this.label3.Margin = new System.Windows.Forms.Padding(3);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(39, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Album:";
            // 
            // label4
            // 
            this.label4.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 60);
            this.label4.Margin = new System.Windows.Forms.Padding(3);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(33, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Time:";
            // 
            // label5
            // 
            this.label5.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(3, 79);
            this.label5.Margin = new System.Windows.Forms.Padding(3);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(50, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Duration:";
            // 
            // TimeLabel
            // 
            this.TimeLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.TimeLabel.AutoSize = true;
            this.TimeLabel.Location = new System.Drawing.Point(59, 60);
            this.TimeLabel.Name = "TimeLabel";
            this.TimeLabel.Size = new System.Drawing.Size(35, 13);
            this.TimeLabel.TabIndex = 5;
            this.TimeLabel.Text = "label6";
            // 
            // DurationLabel
            // 
            this.DurationLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.DurationLabel.AutoSize = true;
            this.DurationLabel.Location = new System.Drawing.Point(59, 79);
            this.DurationLabel.Name = "DurationLabel";
            this.DurationLabel.Size = new System.Drawing.Size(35, 13);
            this.DurationLabel.TabIndex = 6;
            this.DurationLabel.Text = "label7";
            // 
            // TrackTitleLabel
            // 
            this.TrackTitleLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.TrackTitleLabel.AutoSize = true;
            this.TrackTitleLabel.Location = new System.Drawing.Point(59, 3);
            this.TrackTitleLabel.Name = "TrackTitleLabel";
            this.TrackTitleLabel.Size = new System.Drawing.Size(0, 13);
            this.TrackTitleLabel.TabIndex = 7;
            // 
            // TrackArtistLabel
            // 
            this.TrackArtistLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.TrackArtistLabel.AutoSize = true;
            this.TrackArtistLabel.Location = new System.Drawing.Point(59, 22);
            this.TrackArtistLabel.Name = "TrackArtistLabel";
            this.TrackArtistLabel.Size = new System.Drawing.Size(0, 13);
            this.TrackArtistLabel.TabIndex = 8;
            // 
            // AlbumLabel
            // 
            this.AlbumLabel.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.AlbumLabel.AutoSize = true;
            this.AlbumLabel.Location = new System.Drawing.Point(59, 41);
            this.AlbumLabel.Name = "AlbumLabel";
            this.AlbumLabel.Size = new System.Drawing.Size(0, 13);
            this.AlbumLabel.TabIndex = 9;
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(771, 542);
            this.Controls.Add(this.tableLayoutPanel3);
            this.Controls.Add(this.statusStrip1);
            this.MinimumSize = new System.Drawing.Size(787, 581);
            this.Name = "MainWindow";
            this.Text = "Form1";
            this.tableLayoutPanel3.ResumeLayout(false);
            this.tableLayoutPanel3.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel2.ResumeLayout(false);
            this.tableLayoutPanel2.PerformLayout();
            this.tableLayoutPanel4.ResumeLayout(false);
            this.tableLayoutPanel4.PerformLayout();
            this.tableLayoutPanel5.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.SeekBar)).EndInit();
            this.tableLayoutPanel6.ResumeLayout(false);
            this.tableLayoutPanel6.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel3;
        private System.Windows.Forms.ListView PlaylistView;
        private System.Windows.Forms.ColumnHeader PlaylistIndex;
        private System.Windows.Forms.ColumnHeader TrackNo;
        private System.Windows.Forms.ColumnHeader Artist;
        private System.Windows.Forms.ColumnHeader Album;
        private System.Windows.Forms.ColumnHeader TrackName;
        private System.Windows.Forms.ColumnHeader Duration;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Label ArtCoverLabel;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel4;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel5;
        private System.Windows.Forms.Button PreviousTrackButton;
        private System.Windows.Forms.Button LoadButton;
        private System.Windows.Forms.Button PlayButton;
        private System.Windows.Forms.Button NextTrackButton;
        private System.Windows.Forms.Button PauseButton;
        private System.Windows.Forms.Button StopButton;
        private System.Windows.Forms.TrackBar SeekBar;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel6;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label TimeLabel;
        private System.Windows.Forms.Label DurationLabel;
        private System.Windows.Forms.Label TrackTitleLabel;
        private System.Windows.Forms.Label TrackArtistLabel;
        private System.Windows.Forms.Label AlbumLabel;
    }
}

