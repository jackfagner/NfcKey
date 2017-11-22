/*
    Copyright (C) 2017  github.com/jackfagner

    This file is part of NfcKey.

    NfcKey is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NfcKey is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with NfcKey.  If not, see <http://www.gnu.org/licenses/>.
*/
using System;

namespace NfcHack
{
    public static class NfcKey
    {
        private static readonly UInt32[] intConst = {
            0x6D835AFC, 0x7D15CD97, 0x0942B409, 0x32F9C923, 0xA811FB02, 0x64F121E8,
            0xD1CC8B4E, 0xE8873E6F, 0x61399BBB, 0xF1B91926, 0xAC661520, 0xA21A31C9,
            0xD424808D, 0xFE118E07, 0xD18E728D, 0xABAC9E17, 0x18066433, 0x00E18E79,
            0x65A77305, 0x5AE9E297, 0x11FC628C, 0x7BB3431F, 0x942A8308, 0xB2F8FD20,
            0x5728B869, 0x30726D5A
        };

        public static Byte[] GetKey(Byte[] uid)
        {
            if (uid == null || uid.Length != 7)
                throw new ArgumentException("UID must be 7 bytes");
            var uid8 = new Byte[8];
            uid.CopyTo(uid8, 0);
            var rotUid = new Byte[8];
            var rotation = (uid8[1] + uid8[3] + uid8[5] + uid8[7]) & 7;
            for (var i = 0; i <= 7; i++)
                rotUid[(i + rotation) & 7] = uid8[i];

            var transfUid = TransformUid(rotUid);

            UInt32 intKey = 0;
            var offset = (transfUid[0] + transfUid[2] + transfUid[4] + transfUid[6]) & 3;
            for (var i = 0; i < 4; i++)
                intKey = transfUid[i + offset] + (intKey << 8);

            return BitConverter.GetBytes(intKey);
        }

        public static Byte[] GetPack(Byte[] uid)
        {
            if (uid == null || uid.Length != 7)
                throw new ArgumentException("UID must be 7 bytes");
            var uid8 = new Byte[8];
            uid.CopyTo(uid8, 0);
            var rotUid = new Byte[8];
            var rotation = (uid8[2] + uid8[5] + uid8[7]) & 7;
            for (var i = 0; i <= 7; i++)
                rotUid[(i + rotation) & 7] = uid8[i];

            var transfUid = TransformUid(rotUid);

            UInt32 intPack = 0;
            for (var i = 0; i < 8; i++)
                intPack += (UInt32)(transfUid[i] * 13);
            
            var packBytes = BitConverter.GetBytes(intPack ^ 0x5555);
            return new Byte[2] { packBytes[0], packBytes[1] };
        }

        private static Byte[] TransformUid(Byte[] rotUid)
        {
            uint intPos = 0, tmp1, tmp2;
            var value1 = (UInt32)((rotUid[3] << 24) | (rotUid[2] << 16) | (rotUid[1] << 8) | rotUid[0]) + intConst[intPos++];
            var value2 = (UInt32)((rotUid[7] << 24) | (rotUid[6] << 16) | (rotUid[5] << 8) | rotUid[4]) + intConst[intPos++];
            
            for (var i = 0; i < 12; i += 2)
            {
                tmp1 = RotateLeft(value1 ^ value2, value2 & 0x1F) + intConst[intPos++];
                tmp2 = RotateLeft(value2 ^ tmp1, tmp1 & 0x1F) + intConst[intPos++];
                value1 = RotateLeft(tmp1 ^ tmp2, tmp2 & 0x1F) + intConst[intPos++];
                value2 = RotateLeft(tmp2 ^ value1, value1 & 0x1F) + intConst[intPos++];
            }

            var result = new Byte[8];
            BitConverter.GetBytes(value1).CopyTo(result, 0);
            BitConverter.GetBytes(value2).CopyTo(result, 4);
            return result;
        }

        private static UInt32 RotateLeft(UInt32 x, Byte n)
        {
            return (x << n) | (x >> (32 - n));
        }

        private static UInt32 RotateLeft(UInt32 x, UInt32 n)
        {
            return RotateLeft(x, (Byte)n);
        }
    }
}
