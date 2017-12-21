using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace managedapp
{

    enum Days { Sat, Sun, Mon, Tue, Wed, Thu, Fri };

    class Class1
    {
        private int Field1;

        public Class1()
        {
            Field1 = -555;
        }

        public class Nested
        {
            public Nested()
            {
                Field1 = 111;
            }
            private long Field1;
        }
    }


    class TestClassBase
    {
        public long longField;
        public int  intField;

        public TestClassBase()
        {
            longField = 0xAABBCCDD;
        }
    }

    class TestClass : TestClassBase
    {
        private char charField;
        private short shortField;
        private int[] intArray;
        private float[, ,] floatArray;
        private string strField;
        private Days daysField;
        private static string staticStrField = "staticField";

        public Class1 class1Field;

        public TestClass()
        {
            charField = 'a';
            shortField = 3456;
            intArray = new int[4] { 1, 128, -555, 8888 };
            floatArray = new float[2, 2, 2] { { { 0.1f, 0.2f }, { 0.3f, 0.4f } }, { { 0.5f, 0.6f }, { 0.7f, 0.8f } } };
            strField = "Hello";
            class1Field = new Class1();
            daysField = Days.Wed;
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
