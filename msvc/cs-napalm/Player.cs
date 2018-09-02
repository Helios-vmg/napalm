using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace cs_napalm
{
    public class Player : IDisposable
    {
        private struct RationalStruct
        {
            public long numerator, denominator;
        }

        public enum NotificationType
        {
            Nothing = 0,
            Destructing,
            TrackChanged,
            SeekComplete,
            PlaylistUpdated,
	        VolumeChangedBySystem,
        }

        public delegate void OnNotification(Notification notification);

        public struct Notification
        {
            public NotificationType Type;
            public int Param1;
            public long Param2;
            public long Param3;
            public IntPtr Param4;
        }

        private struct PrivateNotification
        {
            public int type;
            public int param1;
            public long param2;
            public long param3;
            public IntPtr param4;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnNotificationPrivate(IntPtr data, ref PrivateNotification notification);

        private struct Callbacks
        {
            public IntPtr cb_data;
            public OnNotificationPrivate on_notification;
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

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReleaseFunction(IntPtr p);

        private struct OutputDeviceList
        {
            public IntPtr opaque;
            public ReleaseFunction release_function;
            public UIntPtr length;
            public IntPtr items;
        }

        private struct PrivateOutputDevice
        {
            public IntPtr name;
            public byte unique_id_00;
            public byte unique_id_01;
            public byte unique_id_02;
            public byte unique_id_03;
            public byte unique_id_04;
            public byte unique_id_05;
            public byte unique_id_06;
            public byte unique_id_07;
            public byte unique_id_08;
            public byte unique_id_09;
            public byte unique_id_10;
            public byte unique_id_11;
            public byte unique_id_12;
            public byte unique_id_13;
            public byte unique_id_14;
            public byte unique_id_15;
            public byte unique_id_16;
            public byte unique_id_17;
            public byte unique_id_18;
            public byte unique_id_19;
            public byte unique_id_20;
            public byte unique_id_21;
            public byte unique_id_22;
            public byte unique_id_23;
            public byte unique_id_24;
            public byte unique_id_25;
            public byte unique_id_26;
            public byte unique_id_27;
            public byte unique_id_28;
            public byte unique_id_29;
            public byte unique_id_30;
            public byte unique_id_31;
        }

        public class OutputDevice
        {
            public string Name;
            public byte[] UniqueId;
            public override string ToString()
            {
                return Name;
            }
        }

        private struct PrivateLevel
        {
            public sbyte level_count;
            public float level_00;
            public float level_01;
            public float level_02;
            public float level_03;
            public float level_04;
            public float level_05;
            public float level_06;
            public float level_07;
            public float level_08;
            public float level_09;
            public float level_10;
            public float level_11;
            public float level_12;
            public float level_13;
            public float level_14;
            public float level_15;
        }

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr napalm_create_player();

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_destroy_player(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern int napalm_load_file(IntPtr player, [MarshalAs(UnmanagedType.LPArray)] byte[] path);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_play(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_pause(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_stop(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_previous(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_next(IntPtr player);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_get_current_time(IntPtr player, ref RationalStruct time, ref PrivateLevel level);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_set_callbacks(IntPtr player, ref Callbacks callbacks);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_get_playlist_state(IntPtr player, out int size, out int position);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr napalm_get_basic_track_info(IntPtr player, int playlist_position);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_release_basic_track_info(IntPtr info);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_seek_to_time(IntPtr player, RationalStruct time);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr napalm_get_front_cover(IntPtr player, int playlist_position, ref int data_size);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_release_front_cover(IntPtr player, IntPtr buffer);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_get_outputs(IntPtr player, ref OutputDeviceList dst);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_get_selected_output(IntPtr player, [MarshalAs(UnmanagedType.LPArray)] byte[] dst);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_select_output(IntPtr player, [MarshalAs(UnmanagedType.LPArray)] byte[] dst);

        [DllImport("napalm", CallingConvention = CallingConvention.Cdecl)]
        private static extern void napalm_set_volume(IntPtr player, double volume);

        private IntPtr _player;

        public Player()
        {
            _player = napalm_create_player();
            if (_player == IntPtr.Zero)
                throw new Exception("Failed to create player.");
        }

        public void Dispose()
        {
            if (_player != IntPtr.Zero)
            {
                napalm_destroy_player(_player);
                _player = IntPtr.Zero;
            }
            ReleaseHandle(ref _onNotificationHandle);
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
            return napalm_load_file(_player, array) != 0;
        }

        public void Play()
        {
            napalm_play(_player);
        }

        public void Pause()
        {
            napalm_pause(_player);
        }

        public void Stop()
        {
            napalm_stop(_player);
        }

        public void PreviousTrack()
        {
            napalm_previous(_player);
        }

        public void NextTrack()
        {
            napalm_next(_player);
        }

        private static Rational ToRational(RationalStruct r)
        {
            return new Rational(r.numerator, r.denominator);
        }

        public Rational GetCurrentTime()
        {
            Rational time;
            float[] levels;
            GetCurrentTime(out time, out levels);
            return time;
        }
        public void GetCurrentTime(out Rational time, out float[] levels)
        {
            var rs = new RationalStruct();
            var pl = new PrivateLevel();
            napalm_get_current_time(_player, ref rs, ref pl);
            time = ToRational(rs);
            if (pl.level_count == 0)
                levels = null;
            else
            {
                levels = new float[pl.level_count];
                var temp = new float[16];
                temp[0] = pl.level_00;
                temp[1] = pl.level_01;
                temp[2] = pl.level_02;
                temp[3] = pl.level_03;
                temp[4] = pl.level_04;
                temp[5] = pl.level_05;
                temp[6] = pl.level_06;
                temp[7] = pl.level_07;
                temp[8] = pl.level_08;
                temp[9] = pl.level_09;
                temp[10] = pl.level_10;
                temp[11] = pl.level_11;
                temp[12] = pl.level_12;
                temp[13] = pl.level_13;
                temp[14] = pl.level_14;
                temp[15] = pl.level_15;
                for (int i = 0; i < pl.level_count; i++)
                    levels[i] = temp[i];
            }
        }

        private OnNotificationPrivate _onNotification;
        private IntPtr _onNotificationHandle;

        private Notification ToNotification(PrivateNotification pn)
        {
            return new Notification
            {
                Type = (NotificationType)pn.type,
                Param1 = pn.param1,
                Param2 = pn.param2,
                Param3 = pn.param3,
                Param4 = pn.param4,
            };
        }
        
        public void SetCallbacks(OnNotification on)
        {
            Callbacks callbacks;
            callbacks.cb_data = IntPtr.Zero;

            var oldOnNotificationHandle = _onNotificationHandle;

            _onNotification = (IntPtr data, ref PrivateNotification notification) => on(ToNotification(notification));

            _onNotificationHandle = GCHandle.ToIntPtr(GCHandle.Alloc(_onNotification));
            callbacks.on_notification = _onNotification;

            napalm_set_callbacks(_player, ref callbacks);

            ReleaseGcHandle(oldOnNotificationHandle);
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
            napalm_get_playlist_state(_player, out size, out position);
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
            var ptr = napalm_get_basic_track_info(_player, position);
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
                napalm_release_basic_track_info(ptr);
            }
        }

        private static RationalStruct ToRationalStruct(Rational q)
        {
            return new RationalStruct
            {
                numerator = q.Numerator,
                denominator = q.Denominator,
            };
        }

        public void Seek(Rational time)
        {
            napalm_seek_to_time(_player, ToRationalStruct(time));
        }

        public byte[] GetFrontCover(int position)
        {
            int size = 0;
            var buffer = napalm_get_front_cover(_player, position, ref size);
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
                napalm_release_front_cover(_player, buffer);
            }
        }

        public List<OutputDevice> GetOutputs()
        {
            var ret = new List<OutputDevice>();
            var outputs = new OutputDeviceList();
            napalm_get_outputs(_player, ref outputs);
            if (outputs.release_function == null)
                return ret;
            try
            {
                if (outputs.length.ToUInt64() > (ulong)int.MaxValue)
                    throw new Exception("Too many outputs.");
                var n = (int)outputs.length;
                for (int i = 0; i < n; i++)
                {
                    var pod = Marshal.PtrToStructure<PrivateOutputDevice>(outputs.items + i * Marshal.SizeOf(typeof(PrivateOutputDevice)));
                    var id = new byte[32];
                    id[0] = pod.unique_id_00;
                    id[1] = pod.unique_id_01;
                    id[2] = pod.unique_id_02;
                    id[3] = pod.unique_id_03;
                    id[4] = pod.unique_id_04;
                    id[5] = pod.unique_id_05;
                    id[6] = pod.unique_id_06;
                    id[7] = pod.unique_id_07;
                    id[8] = pod.unique_id_08;
                    id[9] = pod.unique_id_09;
                    id[10] = pod.unique_id_10;
                    id[11] = pod.unique_id_11;
                    id[12] = pod.unique_id_12;
                    id[13] = pod.unique_id_13;
                    id[14] = pod.unique_id_14;
                    id[15] = pod.unique_id_15;
                    id[16] = pod.unique_id_16;
                    id[17] = pod.unique_id_17;
                    id[18] = pod.unique_id_18;
                    id[19] = pod.unique_id_19;
                    id[20] = pod.unique_id_20;
                    id[21] = pod.unique_id_21;
                    id[22] = pod.unique_id_22;
                    id[23] = pod.unique_id_23;
                    id[24] = pod.unique_id_24;
                    id[25] = pod.unique_id_25;
                    id[26] = pod.unique_id_26;
                    id[27] = pod.unique_id_27;
                    id[28] = pod.unique_id_28;
                    id[29] = pod.unique_id_29;
                    id[30] = pod.unique_id_30;
                    id[31] = pod.unique_id_31;
                    ret.Add(new OutputDevice
                    {
                        Name = Utf8ToString(pod.name),
                        UniqueId = id,
                    });
                }
                return ret;
            }
            finally
            {
                outputs.release_function(outputs.opaque);
            }
        }

        public byte[] GetSelectedOutput()
        {
            var ret = new byte[32];
            napalm_get_selected_output(_player, ret);
            return ret;
        }

        public void SelectOutput(byte[] uniqueId)
        {
            napalm_select_output(_player, uniqueId);
        }

        public void SetVolume(double volume)
        {
            napalm_set_volume(_player, volume);
        }
    }
}
