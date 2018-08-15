using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace cs_napalm
{
    public partial class Form1 : Form
    {
        private Player _player;
        private Timer _timer = new Timer();

        public Form1()
        {
            InitializeComponent();
            _player = new Player();
            _timer.Interval = 100;
            _timer.Tick += UpdateTime;
            _timer.Enabled = true;
        }

        private void LoadButton_Click(object sender, EventArgs e)
        {
            var d = new OpenFileDialog();
            var result = d.ShowDialog();
            if (result == DialogResult.OK)
                _player.LoadFile(d.FileName);
        }

        private void PlayButton_Click(object sender, EventArgs e)
        {
            _player.Play();
        }

        private void PauseButton_Click(object sender, EventArgs e)
        {
            _player.Pause();
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            _player.Stop();
        }

        static string FormatTime(double time)
        {
            bool negative = time < 0;
            if (negative)
                time = -time;
            var milliseconds = (long)Math.Floor(time * 1000) % 1000;
            var total = (long)Math.Floor(time);
            var seconds = total % 60;
            total /= 60;
            var minutes = total % 60;
            total /= 60;
            var hours = total % 24;
            total /= 24;
            var days = total % 7;
            var weeks = total / 7;

            var ret = new StringBuilder();

            bool force = false;

            if (negative)
                ret.Append("-(");

            if (weeks > 0)
            {
                ret.Append(weeks);
                ret.Append(" wk ");
                force = true;
            }

            if (force || days > 0)
            {
                ret.Append(days);
                ret.Append(" d ");
                force = true;
            }

            if (force || hours > 0)
            {
                ret.Append(hours.ToString("D2"));
                ret.Append(":");
            }
            ret.Append(minutes.ToString("D2"));
            ret.Append(":");
            ret.Append(seconds.ToString("D2"));
            if (milliseconds > 0)
            {
                ret.Append(".");
                ret.Append(milliseconds.ToString("D3"));
            }

            if (negative)
                ret.Append(")");

            return ret.ToString();
        }

        static string AbsoluteFormatTime(double time)
        {
            return time < 0 ? "undefined" : FormatTime(time);
        }

        private void UpdateTime(object sender, EventArgs e)
        {
            var time = _player.GetCurrentTime();
            TimeLabel.Text = AbsoluteFormatTime(time.Item1);
            DurationLabel.Text = AbsoluteFormatTime(time.Item2);
        }
    }
}
