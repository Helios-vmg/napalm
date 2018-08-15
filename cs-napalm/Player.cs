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
        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr create_player();

        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern void destroy_player(IntPtr player);

        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern int load_file(IntPtr player, [MarshalAs(UnmanagedType.LPArray)] byte[] path);

        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern void play(IntPtr player);

        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern void pause(IntPtr player);

        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern void stop(IntPtr player);

        private struct Rational
        {
            public long numerator, denominator;
        }

        private struct CurrentTime
        {
            public Rational current_time, total_time;
        }

        [DllImport("napalm64", CallingConvention = CallingConvention.Cdecl)]
        private static extern CurrentTime get_current_time(IntPtr player);

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

        public Tuple<double, double> GetCurrentTime()
        {
            var time = get_current_time(_player);
            return new Tuple<double, double>((double)time.current_time.numerator / (double)time.current_time.denominator, (double)time.total_time.numerator / (double)time.total_time.denominator);
        }
    }
}
