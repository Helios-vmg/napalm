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
            if (SeekBar.Maximum == 0)
            {
                DraggingSeekbar = false;
                return;
            }
            _player.Seek(_currentDuration * new Rational(SeekBar.Value, SeekBar.Maximum));
        }
    }
}
