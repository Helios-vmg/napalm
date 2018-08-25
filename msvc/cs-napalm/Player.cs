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
        private struct RationalStruct
        {
            public long numerator, denominator;
        }
        
        public delegate void OnTrackChanged();
        public delegate void OnSeekComplete();

        private delegate void OnTrackChangedPrivate(IntPtr data);
        private delegate void OnSeekCompletePrivate(IntPtr data);

        private struct Callbacks
        {
            public IntPtr cb_data;
            public OnTrackChangedPrivate on_track_changed;
            public OnSeekCompletePrivate on_seek_complete;
        }
        
        private struct NumericTrackInfo
        {
            public int track_number_int;
            public RationalStruct duration;
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
        private static extern RationalStruct get_current_time(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void set_callbacks(IntPtr player, ref Callbacks callbacks);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void get_playlist_state(IntPtr player, out int size, out int position);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr get_basic_track_info(IntPtr player, int playlist_position);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void release_basic_track_info(IntPtr info);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void seek_to_time(IntPtr player, RationalStruct time);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr get_front_cover(IntPtr player, int playlist_position, ref int data_size);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void release_front_cover(IntPtr player, IntPtr buffer);

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
            ReleaseHandle(ref _onTrackChangedHandle);
            ReleaseHandle(ref _onSeekCompleteHandle);
        }

        public void ReleaseHandle(ref IntPtr handle)
        {
            ReleaseGcHandle(handle);
            handle = IntPtr.Zero;
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

        private static Rational ToRational(RationalStruct r)
        {
            return new Rational(r.numerator, r.denominator);
        }

        public Rational GetCurrentTime()
        {
            var time = get_current_time(_player);
            return ToRational(time);
        }

        private OnTrackChangedPrivate _onTrackChanged;
        private OnSeekCompletePrivate _onSeekComplete;
        private IntPtr _onTrackChangedHandle;
        private IntPtr _onSeekCompleteHandle;

        public void SetCallbacks(OnTrackChanged otc, OnSeekComplete osc)
        {
            Callbacks callbacks;
            callbacks.cb_data = IntPtr.Zero;

            var oldOnTrackChangedHandle = _onTrackChangedHandle;
            var oldOnSeekCompleteHandle = _onSeekCompleteHandle;

            _onTrackChanged = data => otc();
            _onTrackChangedHandle = GCHandle.ToIntPtr(GCHandle.Alloc(_onTrackChanged));
            callbacks.on_track_changed = _onTrackChanged;

            _onSeekComplete = data => osc();
            _onSeekCompleteHandle = GCHandle.ToIntPtr(GCHandle.Alloc(_onSeekComplete));
            callbacks.on_seek_complete = _onSeekComplete;

            set_callbacks(_player, ref callbacks);

            ReleaseGcHandle(oldOnTrackChangedHandle);
            ReleaseGcHandle(oldOnSeekCompleteHandle);
        }

        private static void ReleaseGcHandle(IntPtr p)
        {
            if (p != IntPtr.Zero)
                GCHandle.FromIntPtr(p).Free();
        }

        public struct PlaylistState
        {
            public int Size;
            public int Position;
        }

        public PlaylistState GetPlaylistState()
        {
            int size, position;
            get_playlist_state(_player, out size, out position);
            return new PlaylistState
            {
                Size = size,
                Position = position,
            };
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
                    Duration = ToRational(info.numeric_track_info.duration),
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

        public void Seek(Rational time)
        {
            var rs = new RationalStruct
            {
                numerator = time.Numerator,
                denominator = time.Denominator,
            };
            seek_to_time(_player, rs);
        }

        public byte[] GetFrontCover(int position)
        {
            int size = 0;
            var buffer = get_front_cover(_player, position, ref size);
            if (buffer == IntPtr.Zero)
                return null;
            try
            {
                var ret = new byte[size];
                Marshal.Copy(buffer, ret, 0, size);
                return ret;
            }
            finally
            {
                release_front_cover(_player, buffer);
            }
        }
    }
}
