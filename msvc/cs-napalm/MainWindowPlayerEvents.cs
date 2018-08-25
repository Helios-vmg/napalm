using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace cs_napalm
{
    public partial class MainWindow : Form
    {
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
                case Player.NotificationType.PlaylistUpdated:
                    OnPlaylistUpdated((int)notification.Param2, (int)notification.Param3);
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

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
                    ArtCoverLabel.Image = ResizeBitmap(bmp, ArtCoverLabel.Size);
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

        private void OnPlaylistUpdated(int beginIndex, int endIndex)
        {
            if (this.FromMainThread(() => OnPlaylistUpdated(beginIndex, endIndex)))
                return;
            var state = _player.GetPlaylistState();

            var e = Math.Max(endIndex, PlaylistView.Items.Count);
            for (int i = beginIndex; i < e; i++)
            {
                if (i >= state.Size)
                    PlaylistView.Items.RemoveAt(state.Size);
                else
                {
                    var info = _player.GetBasicTrackInfo(i);
                    var item = new ListViewItem();
                    item.Text = (i + 1).ToString();
                    item.SubItems.Add(info.TrackNumber);
                    item.SubItems.Add(info.TrackArtist);
                    item.SubItems.Add(info.Album);
                    item.SubItems.Add(info.TrackTitle);
                    item.SubItems.Add(Utility.AbsoluteFormatTime(info.Duration));
                    if (i >= PlaylistView.Items.Count)
                        PlaylistView.Items.Add(item);
                    else
                        PlaylistView.Items[i] = item;
                }
            }
        }

    }
}
