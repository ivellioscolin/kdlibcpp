using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace managedapp
{

    class Class1
    {
        private int Field1;

        public class Nested
        {
            public Nested()
            {
                Field1 = 111;
            }
            private long Field1;
        }
    }

    class Program
    {
        public static int g_int = 10;
        static void Main(string[] args)
        {
            Class1.Nested obj = new Class1.Nested();
            Debugger.Break();
        }
    }
}
