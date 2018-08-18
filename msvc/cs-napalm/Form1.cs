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
    public partial class Form1 : Form
    {
        private Player _player;
        private Timer _timer = new Timer();

        public Form1()
        {
            InitializeComponent();

            ArtCoverLabel.Image = new Bitmap(Resources.napalm_icon, new Size(ArtCoverLabel.Width, ArtCoverLabel.Height));
            
            _timer.Interval = 250;
            _timer.Tick += UpdateTime;
            _timer.Enabled = true;
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

        private void UpdateTime(object sender, EventArgs e)
        {
            if (_player == null)
            {
                TimeLabel.Text = Utility.AbsoluteFormatTime(-1);
                DurationLabel.Text = Utility.AbsoluteFormatTime(-1);
                return;
            }
            var time = _player.GetCurrentTime();
            TimeLabel.Text = Utility.AbsoluteFormatTime(time);
            DurationLabel.Text = Utility.AbsoluteFormatTime(-1);

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
            {
                MessageBox.Show("Track changed, no metadata.");
            }
            else
            {
                MessageBox.Show($"Track changed. Title: {info.TrackTitle}, Duration: {Utility.AbsoluteFormatTime(info.Duration, true)}");
            }
        }

    }
}
