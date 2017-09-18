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

    class TestClass
    {
        private char charField;
        private short shortField;
        private int[] intArray;
        private float[, ,] floatArray;

        public Class1 class1Field;

        public TestClass()
        {
            charField = 'a';
            shortField = 3456;
            intArray = new int[4] { 1, 128, -555, 8888 };
            floatArray = new float[2, 2, 2] { { { 0.1f, 0.2f }, { 0.3f, 0.4f } }, { { 0.5f, 0.6f }, { 0.7f, 0.8f } } };
        }
    }

    class Program
    {
        public static int g_int = 10;
        static void Main(string[] args)
        {
            Class1.Nested  obj = new Class1.Nested();
            TestClass testClass = new TestClass();
            Debugger.Break();
        }
    }
}
