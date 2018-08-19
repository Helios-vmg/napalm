using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_napalm
{
    public class Rational
    {
        public long Numerator { get; private set; }
        public long Denominator { get; private set; }

        public Rational(long n = 0, long d = 1)
        {
            Numerator = n;
            Denominator = d;
            Reduce();
        }

        private static long Gcd(long a, long b)
        {
            if (a < 0)
                a = -a;
            if (b < 0)
                b = -b;
            while (b > 0)
            {
                var temp = b;
                b = a % b;
                a = temp;
            }
            return a;
        }

        private static long Lcm(long a, long b)
        {
            return a*b/Gcd(a, b);
        }

        private void Reduce()
        {
            var c = Gcd(Numerator, Denominator);
            Numerator /= c;
            Denominator /= c;
            if (Denominator < 0)
            {
                Numerator = -Numerator;
                Denominator = -Denominator;
            }
        }

        public Rational Reciprocal()
        {
            return new Rational(Denominator, Numerator);
        }

        public static Rational operator+(Rational a, Rational b)
        {
            return new Rational(a.Numerator * b.Denominator + b.Numerator * a.Denominator, a.Denominator * b.Denominator);
        }
        
        public static Rational operator-(Rational a, Rational b)
        {
            return new Rational(a.Numerator * b.Denominator - b.Numerator * a.Denominator, a.Denominator * b.Denominator);
        }

        public static Rational operator-(Rational a)
        {
            return new Rational(-a.Numerator, a.Denominator);
        }

        public static Rational operator*(Rational a, Rational b)
        {
            return new Rational(a.Numerator * b.Numerator, a.Denominator * b.Denominator);
        }

        public static Rational operator*(Rational a, long b)
        {
            return new Rational(a.Numerator * b, a.Denominator);
        }

        public static Rational operator/(Rational a, Rational b)
        {
            return new Rational(a.Numerator * b.Denominator, a.Denominator * b.Numerator);
        }

        public static Rational operator/(Rational a, long b)
        {
            return new Rational(a.Numerator, a.Denominator * b);
        }

        public static bool operator<(Rational a, Rational b)
        {
            return a.Numerator * b.Denominator < b.Numerator * a.Denominator;
        }

        public static bool operator<(Rational a, long b)
        {
            return a.Numerator < b * a.Denominator;
        }

        public static bool operator>(Rational a, Rational b)
        {
            return a.Numerator * b.Denominator > b.Numerator * a.Denominator;
        }

        public static bool operator>(Rational a, long b)
        {
            return a.Numerator > b * a.Denominator;
        }

        public static bool operator<=(Rational a, Rational b)
        {
            return a.Numerator * b.Denominator <= b.Numerator * a.Denominator;
        }

        public static bool operator<=(Rational a, long b)
        {
            return a.Numerator <= b * a.Denominator;
        }

        public static bool operator>=(Rational a, Rational b)
        {
            return a.Numerator * b.Denominator >= b.Numerator * a.Denominator;
        }

        public static bool operator>=(Rational a, long b)
        {
            return a.Numerator >= b * a.Denominator;
        }

        public static bool operator==(Rational a, Rational b)
        {
            return a.Numerator == b.Numerator && a.Denominator == b.Denominator;
        }

        public static bool operator==(Rational a, long b)
        {
            return a.Numerator == b && a.Denominator == 1;
        }

        public static bool operator!=(Rational a, Rational b)
        {
            return a.Numerator != b.Numerator || a.Denominator != b.Denominator;
        }

        public static bool operator!=(Rational a, long b)
        {
            return a.Numerator != b || a.Denominator != 1;
        }

        protected bool Equals(Rational other)
        {
            return Numerator == other.Numerator && Denominator == other.Denominator;
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            if (obj.GetType() != this.GetType()) return false;
            return Equals((Rational)obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                return (Numerator.GetHashCode() * 397) ^ Denominator.GetHashCode();
            }
        }

        public double ToDouble()
        {
            return (double) Numerator/(double) Denominator;
        }

        public long ToLongTruncate()
        {
            return Numerator / Denominator;
        }

        public int ToIntTruncate()
        {
            return (int)ToLongTruncate();
        }

        public string ToString()
        {
            return $"{Numerator}/{Denominator}";
        }
    }
}
