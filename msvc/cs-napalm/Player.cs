using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace cs_napalm
{
    class Player : IDisposable
    {
        private struct Rational
        {
            public long numerator, denominator;
        }
        
        public delegate void OnTrackChanged();

        private delegate void OnTrackChangedPrivate(IntPtr data);

        private struct Callbacks
        {
            public IntPtr cb_data;
            public OnTrackChangedPrivate on_track_changed;
        }
        
        private struct NumericTrackInfo
        {
            public int track_number_int;
            public Rational duration;
            public double track_gain;
            public double track_peak;
            public double album_gain;
            public double album_peak;
        }

        private struct PrivateBasicTrackInfo
        {
            public IntPtr album;
            public IntPtr track_title;
            public IntPtr track_number;
            public IntPtr track_artist;
            public IntPtr date;
            public IntPtr track_id;
            public IntPtr path;
            public NumericTrackInfo numeric_track_info;
        }

        public class BasicTrackInfo
        {
            public string Album;
            public string TrackTitle;
            public string TrackNumber;
            public string TrackArtist;
            public string Date;
            public string TrackId;
            public string Path;
            public int TrackNumberInt;
            public double Duration;
            public double TrackGain;
            public double TrackPeak;
            public double AlbumGain;
            public double AlbumPeak;
        }

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr create_player();

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void destroy_player(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern int load_file(IntPtr player, [MarshalAs(UnmanagedType.LPArray)] byte[] path);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void play(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void pause(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void stop(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void previous(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void next(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern Rational get_current_time(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void set_callbacks(IntPtr player, ref Callbacks callbacks);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void get_playlist_state(IntPtr player, out int size, out int position);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr get_basic_track_info(IntPtr player, int playlist_position);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void release_basic_track_info(IntPtr info);

        private IntPtr _player;

        public Player()
        {
            _player = create_player();
            if (_player == IntPtr.Zero)
                throw new Exception("Failed to create player.");
        }

        public void Dispose()
        {
            if (_player != IntPtr.Zero)
            {
                destroy_player(_player);
                _player = IntPtr.Zero;
            }
            ReleaseGcHandle(_onTrackChangedHandle);
            _onTrackChangedHandle = IntPtr.Zero;
        }

        public bool LoadFile(string path)
        {
            var tempList = Encoding.UTF8.GetBytes(path).ToList();
            tempList.Add(0);
            var array = tempList.ToArray();
            return load_file(_player, array) != 0;
        }

        public void Play()
        {
            play(_player);
        }

        public void Pause()
        {
            pause(_player);
        }

        public void Stop()
        {
            stop(_player);
        }

        public void PreviousTrack()
        {
            previous(_player);
        }

        public void NextTrack()
        {
            next(_player);
        }

        public double GetCurrentTime()
        {
            var time = get_current_time(_player);
            return (double)time.numerator / (double)time.denominator;
        }

        private OnTrackChangedPrivate _onTrackChanged;
        private IntPtr _onTrackChangedHandle;

        public void SetCallbacks(OnTrackChanged otc)
        {
            Callbacks callbacks;
            callbacks.cb_data = IntPtr.Zero;

            var oldOnTrackChangedHandle = _onTrackChangedHandle;
            _onTrackChanged = data => otc();
            _onTrackChangedHandle = GCHandle.ToIntPtr(GCHandle.Alloc(_onTrackChanged));
            callbacks.on_track_changed = _onTrackChanged;

            set_callbacks(_player, ref callbacks);

            ReleaseGcHandle(oldOnTrackChangedHandle);
        }

        private static void ReleaseGcHandle(IntPtr p)
        {
            if (p != IntPtr.Zero)
                GCHandle.FromIntPtr(p).Free();
        }

        public Tuple<int, int> GetPlaylistState()
        {
            int size, position;
            get_playlist_state(_player, out size, out position);
            return new Tuple<int, int>(size, position);
        }

        private static string Utf8ToString(IntPtr ptr)
        {
            int size = 0;
            while (Marshal.ReadByte(ptr, size) != 0)
                size++;
            var buffer = new byte[size];
            Marshal.Copy(ptr, buffer, 0, buffer.Length);
            return Encoding.UTF8.GetString(buffer);
        }
        
        public BasicTrackInfo GetBasicTrackInfo(int position)
        {
            var ptr = get_basic_track_info(_player, position);
            if (ptr == IntPtr.Zero)
                return null;
            try
            {
                var info = (PrivateBasicTrackInfo)Marshal.PtrToStructure(ptr, typeof (PrivateBasicTrackInfo));
                return new BasicTrackInfo
                {
                    Album = Utf8ToString(info.album),
                    TrackTitle = Utf8ToString(info.track_title),
                    TrackNumber = Utf8ToString(info.track_number),
                    TrackArtist = Utf8ToString(info.track_artist),
                    Date = Utf8ToString(info.date),
                    TrackId = Utf8ToString(info.track_id),
                    Path = Utf8ToString(info.path),
                    TrackNumberInt = info.numeric_track_info.track_number_int,
                    Duration = (double)info.numeric_track_info.duration.numerator / (double)info.numeric_track_info.duration.denominator,
                    TrackGain = info.numeric_track_info.track_gain,
                    TrackPeak = info.numeric_track_info.track_peak,
                    AlbumGain = info.numeric_track_info.album_gain,
                    AlbumPeak = info.numeric_track_info.album_peak,
                };
            }
            finally
            {
                release_basic_track_info(ptr);
            }
        }
    }
}
