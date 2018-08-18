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
    }
}
