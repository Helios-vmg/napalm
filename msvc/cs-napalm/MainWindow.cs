using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
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
        private Bitmap DefaultCover;

        public MainWindow()
        {
            InitializeComponent();

            DefaultCover = new Bitmap(Resources.napalm_icon, new Size(ArtCoverLabel.Width, ArtCoverLabel.Height));
            DisplayDefaultCover();

            _timer.Interval = HighPrecisionTime ? 10 : 250;
            _timer.Tick += (sender, args) => OnTick();
            _timer.Enabled = true;
        }

        public void DisplayDefaultCover()
        {
            ArtCoverLabel.Image = DefaultCover;
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
            _player.SetCallbacks(OnPlayerNotification);
        }

        private void OnPlayerNotification(Player.Notification notification)
        {
            switch (notification.Type)
            {
                case Player.NotificationType.Nothing:
                case Player.NotificationType.Destructing:
                    break;
                case Player.NotificationType.TrackChanged:
                    OnTrackChanged();
                    break;
                case Player.NotificationType.SeekComplete:
                    OnSeekComplete();
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }
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
            SetDuration(new Rational(0));
            UpdateTime(new Rational(0), false, false);
            DisplayDefaultCover();
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

        private void OnTick()
        {
            UpdateTime(new Rational(-1), false, true);
        }

        private void UpdateTime(Rational overrideValue, bool hpt, bool timed)
        {
            if (_player == null)
            {
                TimeLabel.Text = Utility.AbsoluteFormatTime(-1);
                DurationLabel.Text = Utility.AbsoluteFormatTime(-1);
                return;
            }
            var time = overrideValue < 0 ? _player.GetCurrentTime() : overrideValue;
            if (timed && !DraggingSeekbar)
                TimeLabel.Text = Utility.AbsoluteFormatTime(time.ToDouble(), DraggingSeekbar || hpt || HighPrecisionTime);

            var position = (time*100).ToIntTruncate();
            var duration = (_currentDuration*100).ToIntTruncate();
            if (!DraggingSeekbar)
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

        private void SetBasicMetadata(BasicTrackInfo info)
        {
            TrackTitleLabel.Text = info.TrackTitle;
            TrackArtistLabel.Text = info.TrackArtist;
            AlbumLabel.Text = info.Album;
            SetDuration(info.Duration);
            UpdateTime(new Rational(0), false, false);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void SeekBar_Scroll(object sender, EventArgs e)
        {
            if (SeekBar.Maximum > 0)
                UpdateTime(new Rational(SeekBar.Value, SeekBar.Maximum), true, false);
        }

        private void SeekBar_MouseDown(object sender, MouseEventArgs e)
        {
            DraggingSeekbar = true;
        }

        private void SeekBar_MouseUp(object sender, MouseEventArgs e)
        {
            InitPlayer();
            _player.Seek(_currentDuration*new Rational(SeekBar.Value, SeekBar.Maximum));
        }

        #region Player Callbacks

        private void OnTrackChanged()
        {
            if (this.FromMainThread(OnTrackChanged))
                return;
            var state = _player.GetPlaylistState();
            var info = _player.GetBasicTrackInfo(state.Position);
            if (info != null)
                SetBasicMetadata(info);
            var cover = _player.GetFrontCover(state.Position);
            if (cover != null)
            {
                using (var mem = new MemoryStream(cover))
                {
                    var bmp = new Bitmap(mem);
                    ArtCoverLabel.Image = new Bitmap(bmp, new Size(ArtCoverLabel.Width, ArtCoverLabel.Height));
                }
            }
        }

        private void OnSeekComplete()
        {
            if (this.FromMainThread(OnSeekComplete))
                return;
            if (!DraggingSeekbar)
                return;
            DraggingSeekbar = false;
            UpdateTime(new Rational(-1), false, false);
        }

        #endregion
    }
}
