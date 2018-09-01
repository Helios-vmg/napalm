using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace cs_napalm
{
    public partial class MainWindow : Form
    {
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
            SetDuration(new Rational(-1));
            UpdateTime(new Rational(0), false);
            LevelMeter.LevelChanged(null);
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

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        ToolTip seekBarTooltip = new ToolTip();

        private void SeekBar_Scroll(object sender, EventArgs e)
        {
            if (SeekBar.Maximum > 0)
            {
                var seekBarRatio = new Rational(SeekBar.Value, SeekBar.Maximum);
                UpdateTime(seekBarRatio, false);
                SetSeekBarToolTip(seekBarRatio*_currentDuration);
            }
        }

        private void SeekBar_MouseDown(object sender, MouseEventArgs e)
        {
            seekBarTooltip.InitialDelay = 0;
            seekBarTooltip.ReshowDelay = 0;
            seekBarTooltip.UseAnimation = false;
            seekBarTooltip.Active = true;
            var seekBarRatio = new Rational(SeekBar.Value, SeekBar.Maximum);
            SetSeekBarToolTip(seekBarRatio*_player.GetCurrentTime());

            DraggingSeekbar = true;
        }

        private void SeekBar_MouseUp(object sender, MouseEventArgs e)
        {
            InitPlayer();
            seekBarTooltip.Active = false;
            if (SeekBar.Maximum == 0)
            {
                DraggingSeekbar = false;
                return;
            }
            _player.Seek(_currentDuration*new Rational(SeekBar.Value, SeekBar.Maximum));
        }

        private void preferencesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            InitPlayer();
            var dialog = new PreferencesDialog(_player);
            dialog.ShowDialog(this);
        }

        ToolTip volumeTooltip = new ToolTip();
        //Note: True linear control to dB conversion would use a LogFactor of 8.685889638065035.
        private const double LogFactor = 14.42695130086724;

        private double VolumeControlValue
        {
            get
            {
                var vol = (double) VolumeControl.Value/VolumeControl.Maximum;
                return Math.Log(vol) * LogFactor;
            }
            set
            {
                VolumeControl.Value = (int) (Math.Exp(value/LogFactor)*VolumeControl.Maximum);
            }
        }

        private int _volumeRecursion = 0;

        private void volumeControl_ValueChanged(object sender, EventArgs e)
        {
            if (_volumeRecursion != 0)
                return;
            _volumeRecursion++;
            try
            {
                var volume = VolumeControlValue;
                SetVolumeToolTip(volume);
                _player.SetVolume(volume);
            }
            finally
            {
                _volumeRecursion--;
            }
        }

        private bool _draggingVolume = false;

        private void VolumeControl_MouseDown(object sender, MouseEventArgs e)
        {
            volumeTooltip.InitialDelay = 0;
            volumeTooltip.ReshowDelay = 0;
            volumeTooltip.UseAnimation = false;
            SetVolumeToolTip(VolumeControlValue);
            volumeTooltip.Active = true;
            _draggingVolume = true;
        }

        private void VolumeControl_MouseUp(object sender, MouseEventArgs e)
        {
            volumeTooltip.Active = false;
            _draggingVolume = false;
        }
    }
}
