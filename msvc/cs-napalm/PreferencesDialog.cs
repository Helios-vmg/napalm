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
    public partial class PreferencesDialog : Form
    {
        private Player _player;
        public PreferencesDialog(Player player)
        {
            _player = player;
            InitializeComponent();
            var outputs = _player.GetOutputs();
            if (outputs.Count == 0)
            {
                OutputsBox.Items.Add(new Player.OutputDevice
                {
                    Name = "No outputs",
                    UniqueId = new byte[32],
                });
                OutputsBox.Enabled = false;
                OutputsBox.SelectedIndex = 0;
                return;
            }

            OutputsBox.Items.AddRange(outputs.ToArray());
            var selected = _player.GetSelectedOutput();
            int found = -1;
            for (int i = 0; i < outputs.Count; i++)
            {
                if (selected.Equals(outputs[i].UniqueId))
                {
                    found = i;
                    break;
                }
            }
            OutputsBox.SelectedIndex = found;
        }

        private void OkButton_Click(object sender, EventArgs e)
        {
            Apply();
            Close();
        }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void Apply()
        {
            var index = OutputsBox.SelectedIndex;
            if (index < 0)
                return;
            var dev = (Player.OutputDevice) OutputsBox.Items[index];
            _player.SelectOutput(dev.UniqueId);
        }
    }
}
