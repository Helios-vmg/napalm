using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Text;
using System.Linq;
using System.Runtime.CompilerServices;
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
        private const bool HighPrecisionTime = false;
        private Rational _currentDuration = new Rational(-1);
        private bool DraggingSeekbar = false;

        public MainWindow()
        {
            InitializeComponent();

            ArtCoverLabel.Image = new Bitmap(Resources.napalm_icon, new Size(ArtCoverLabel.Width, ArtCoverLabel.Height));
            
            _timer.Interval = HighPrecisionTime ? 10 : 250;
            _timer.Tick += (sender, args) => UpdateTime();
            _timer.Enabled = true;
        }

        public void Leaving()
        {
            _player?.Dispose();
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
            _player.SetCallbacks(OnTrackChanged, OnSeekComplete);
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

        private void UpdateTime()
        {
            UpdateTime(new Rational(-1));
        }

        private void UpdateTime(Rational overrideValue)
        {
            if (_player == null)
            {
                TimeLabel.Text = Utility.AbsoluteFormatTime(-1);
                DurationLabel.Text = Utility.AbsoluteFormatTime(-1);
                return;
            }
            var time = _player.GetCurrentTime();
            if (overrideValue < 0)
            {
                if (!DraggingSeekbar)
                    TimeLabel.Text = Utility.AbsoluteFormatTime(time.ToDouble(), HighPrecisionTime);
            }
            else
                TimeLabel.Text = Utility.AbsoluteFormatTime(_currentDuration*overrideValue, true);

            var position = (time*100).ToIntTruncate();
            var duration = (_currentDuration*100).ToIntTruncate();
            if (overrideValue < 0 && !DraggingSeekbar)
            {
                if (position < 0)
                    SeekBar.Value = 0;
                else if (position <= duration)
                    SeekBar.Value = position;
            }
        }

        private void SetDuration(Rational durationQ)
        {
            _currentDuration = durationQ;
            DurationLabel.Text = Utility.AbsoluteFormatTime(_currentDuration, HighPrecisionTime);
            if (durationQ >= 0)
            {
                SeekBar.Enabled = true;
                var durationI = (_currentDuration*100).ToIntTruncate();
                if (durationI >= 0)
                    SeekBar.TickFrequency = SeekBar.Maximum = durationI;
            }
            else
            {
                SeekBar.Enabled = false;
                SeekBar.Value = 0;
            }
        }

        private void OnTrackChanged()
        {
            var state = _player.GetPlaylistState();
            var info = _player.GetBasicTrackInfo(state.Item2);
            if (info == null)
                return;

            SetBasicMetadata(info);
        }

        private delegate void SetBasicMetadataDelegate(BasicTrackInfo info);

        private void SetBasicMetadata(BasicTrackInfo info)
        {
            if (TrackTitleLabel.InvokeRequired)
            {
                TrackArtistLabel.BeginInvoke(new SetBasicMetadataDelegate(SetBasicMetadata), info);
                return;
            }
            TrackTitleLabel.Text = info.TrackTitle;
            TrackArtistLabel.Text = info.TrackArtist;
            AlbumLabel.Text = info.Album;
            SetDuration(info.Duration);
            UpdateTime();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void SeekBar_Scroll(object sender, EventArgs e)
        {
            if (SeekBar.Maximum > 0)
                UpdateTime(new Rational(SeekBar.Value, SeekBar.Maximum));
        }

        private void SeekBar_MouseDown(object sender, MouseEventArgs e)
        {
            DraggingSeekbar = true;
        }

        private delegate void OnSeekCompleteDelegate();

        private void OnSeekComplete()
        {
            if (TrackTitleLabel.InvokeRequired)
            {
                TrackArtistLabel.BeginInvoke(new OnSeekCompleteDelegate(OnSeekComplete));
                return;
            }
            DraggingSeekbar = false;
            UpdateTime();
        }

        private void SeekBar_MouseUp(object sender, MouseEventArgs e)
        {
            InitPlayer();
            _player.Seek(_currentDuration * new Rational(SeekBar.Value, SeekBar.Maximum));
        }
    }
}
