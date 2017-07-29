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

        class Nested
        {
            private long Field1;
        }
    }

    class Program
    {
        public static int g_int = 10;

        static void Main(string[] args)
        {
            Debugger.Break();
        }
    }
}
