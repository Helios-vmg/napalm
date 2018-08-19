using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Text;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using cs_napalm.Properties;

namespace cs_napalm
{
    public partial class MainWindow : Form
    {
        private Player _player;
        private Timer _timer = new Timer();
        const bool HighPrecisionTime = false;

        public MainWindow()
        {
            InitializeComponent();

            ArtCoverLabel.Image = new Bitmap(Resources.napalm_icon, new Size(ArtCoverLabel.Width, ArtCoverLabel.Height));
            
            _timer.Interval = HighPrecisionTime ? 10 : 250;
            _timer.Tick += UpdateTime;
            _timer.Enabled = true;
        }

        public void Leaving()
        {
            _player.Dispose();
        }

        private void LoadButton_Click(object sender, EventArgs e)
        {
            var d = new OpenFileDialog();
            var result = d.ShowDialog();
            if (result == DialogResult.OK)
            {
                InitPlayer();
                _player.LoadFile(d.FileName);
            }
        }

        void InitPlayer()
        {
            if (_player != null)
                return;
            _player = new Player();
            _player.SetCallbacks(OnTrackChanged);
        }

        private void PlayButton_Click(object sender, EventArgs e)
        {
            InitPlayer();
            _player.Play();
        }

        private void PauseButton_Click(object sender, EventArgs e)
        {
            InitPlayer();
            _player.Pause();
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            InitPlayer();
            _player.Stop();
        }

        private void PreviousTrackButton_Click(object sender, EventArgs e)
        {
            InitPlayer();
            _player.PreviousTrack();
        }

        private void NextTrackButton_Click(object sender, EventArgs e)
        {
            InitPlayer();
            _player.NextTrack();
        }

        private void UpdateTime(object sender = null, EventArgs e = null)
        {
            if (_player == null)
            {
                TimeLabel.Text = Utility.AbsoluteFormatTime(-1);
                DurationLabel.Text = Utility.AbsoluteFormatTime(-1);
                return;
            }
            var time = _player.GetCurrentTime();
            TimeLabel.Text = Utility.AbsoluteFormatTime(time, HighPrecisionTime);
            DurationLabel.Text = Utility.AbsoluteFormatTime(-1, HighPrecisionTime);

            var position = (int) Math.Floor(time*100);
            var duration = (int) Math.Floor(-100.0);
            if (duration >= 0)
                SeekBar.TickFrequency = SeekBar.Maximum = duration;
            if (position >= 0 && position <= duration)
                SeekBar.Value = position;
        }

        private void OnTrackChanged()
        {
            var state = _player.GetPlaylistState();
            var info = _player.GetBasicTrackInfo(state.Item2);
            if (info == null)
                return;

            SetBasicMetadata(info);
            UpdateTime();
        }

        private delegate void SetBasicMetadataDelegate(BasicTrackInfo info);

        private void SetBasicMetadata(BasicTrackInfo info)
        {
            if (TrackTitleLabel.InvokeRequired)
            {
                TrackArtistLabel.Invoke(new SetBasicMetadataDelegate(SetBasicMetadata), info);
                return;
            }
            TrackTitleLabel.Text = info.TrackTitle;
            TrackArtistLabel.Text = info.TrackArtist;
            AlbumLabel.Text = info.Album;
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
