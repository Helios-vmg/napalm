using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_napalm
{
    static class Utility
    {
        public static string FormatTime(double time, bool ms = false)
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
            if (ms && milliseconds > 0)
            {
                ret.Append(".");
                ret.Append(milliseconds.ToString("D3"));
            }

            if (negative)
                ret.Append(")");

            return ret.ToString();
        }

        public static string AbsoluteFormatTime(double time, bool ms = false)
        {
            return time < 0 ? "N/A" : FormatTime(time, ms);
        }

        public static string AbsoluteFormatTime(Rational time, bool ms = false)
        {
            return AbsoluteFormatTime(time.ToDouble(), ms);
        }

    }
}
