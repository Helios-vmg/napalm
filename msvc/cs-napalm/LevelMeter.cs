using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace cs_napalm
{
    public partial class LevelMeter : UserControl
    {
        private ILevelSource _levelSource;
        public LevelMeter()
        {
            InitializeComponent();
            SetStyle(ControlStyles.UserPaint | ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer, true);
        }

        private static Point Margin = new Point(5, 13);
        private float[] _levels;

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            using (var brush = new SolidBrush(Color.White))
                e.Graphics.FillRectangle(brush, new Rectangle(new Point(0, 0), Size));

            var s = new Size(Size.Width - Margin.X * 2, Size.Height - Margin.Y * 2);
            using (var brush = new SolidBrush(Color.Gray))
            {
                for (int i = 0; i <= 7; i++)
                {
                    e.Graphics.FillRectangle(brush, new Rectangle(new Point(0, Margin.Y + s.Height * ((1 << i) - 1) / (1 << i)), new Size(Margin.X, 1)));
                    e.Graphics.FillRectangle(brush, new Rectangle(new Point(Width - Margin.X, Margin.Y + s.Height * ((1 << i) - 1) / (1 << i)), new Size(Margin.X, 1)));
                }
            }

            int v = 0,
                m = 65536;
            if (_levelSource != null && _levels == null)
                _levels = _levelSource.GetLevels();
            if (_levels != null && _levels.Length != 0)
                v = (int)(_levels[0] * 65536);
            var p = s.Height * (m - v) / m;
            var n = s.Height - p;
            for (int i = 0; i < n; i++)
            {
                int g = 95 - 95*(i + p)/s.Height;
                int b = 191 - 191 * (i + p) / s.Height;
                using (var brush = new SolidBrush(Color.FromArgb(0, g, b)))
                {
                    e.Graphics.FillRectangle(brush,
                        new Rectangle(new Point(Margin.X, Margin.Y + p + i), new Size(s.Width, 1)));
                }
            }

            using (var pen = new Pen(Color.Black))
                e.Graphics.DrawRectangle(pen, new Rectangle(new Point(0, 0), new Size(Size.Width - 1, Size.Height - 1)));

            _levels = null;
        }

        public ILevelSource LevelSource
        {
            set
            {
                _levelSource = value;
                _levelSource.RegisterCallback(LevelChanged);
            }
        }

        public void LevelChanged(float[] levels)
        {
            _levels = levels;
            Invalidate();
            Update();
            //Refresh();
        }
    }
}
