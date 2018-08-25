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

            DefaultCover = ResizeBitmap(Resources.napalm_icon, ArtCoverLabel.Size);
            DisplayDefaultCover();

            _timer.Interval = HighPrecisionTime ? 10 : 250;
            _timer.Tick += (sender, args) => OnTick();
            _timer.Enabled = true;
        }

        public static Bitmap ResizeBitmap(Bitmap src, Size dstSize)
        {
            var srcSize = src.Size;
            if (dstSize == srcSize)
                return src;

            var srcRatio = new Rational(srcSize.Width, srcSize.Height);
            var dstRatio = new Rational(dstSize.Width, dstSize.Height);
            if (srcRatio < dstRatio)
                dstSize = new Size(src.Width * dstSize.Height / src.Height, dstSize.Height);
            else if (srcRatio > dstRatio)
                dstSize = new Size(dstSize.Width, src.Height * dstSize.Width / src.Width);
            return new Bitmap(src, dstSize);
        }

        public void DisplayDefaultCover()
        {
            ArtCoverLabel.Image = DefaultCover;
        }

        public void Leaving()
        {
            _player?.Dispose();
        }

        void InitPlayer()
        {
            if (_player != null)
                return;
            _player = new Player();
            _player.SetCallbacks(OnPlayerNotification);
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
    }
}
