from pathlib import Path
import ctypes
import tkinter as tk
import math
import os
import tempfile


# Formats supported by this script
SUPPORTED_EXTS = {
    ".png", ".gif", ".ppm", ".pgm", ".jpg", ".jpeg"
}

JPEG_EXTS = {".jpg", ".jpeg"}

# GDI+ pixel format: PixelFormat24bppRGB
PIXEL_FORMAT_24BPP_RGB = 0x00021808

# GDI+ InterpolationModeHighQualityBilinear
INTERPOLATION_HIGH_QUALITY_BILINEAR = 7

# GDI+ EncoderParameterValueTypeLong
ENCODER_PARAMETER_TYPE_LONG = 4

# GDI+ JPEG encoder CLSID:
# {557CF401-1A04-11D3-9A73-0000F81EF32E}


class GUID(ctypes.Structure):
    _fields_ = [
        ("Data1", ctypes.c_uint32),
        ("Data2", ctypes.c_uint16),
        ("Data3", ctypes.c_uint16),
        ("Data4", ctypes.c_ubyte * 8),
    ]


class GdiplusStartupInput(ctypes.Structure):
    _fields_ = [
        ("GdiplusVersion", ctypes.c_uint32),
        ("DebugEventCallback", ctypes.c_void_p),
        ("SuppressBackgroundThread", ctypes.c_int),
        ("SuppressExternalCodecs", ctypes.c_int),
    ]


class EncoderParameter(ctypes.Structure):
    _fields_ = [
        ("Guid", GUID),
        ("NumberOfValues", ctypes.c_uint32),
        ("Type", ctypes.c_uint32),
        ("Value", ctypes.c_void_p),
    ]


class EncoderParameters(ctypes.Structure):
    _fields_ = [
        ("Count", ctypes.c_uint32),
        ("Parameter", EncoderParameter * 1),
    ]


def check_gdiplus(status, operation):
    """Raise an exception if a GDI+ operation fails."""
    if status != 0:
        raise RuntimeError(
            f"{operation} failed with GDI+ status {status}."
        )


def resize_jpeg_gdiplus(path, scale):
    """
    Resize a JPEG using Windows GDI+.

    No third-party libraries are required.
    The original file is replaced only after the resized JPEG
    has been saved successfully.
    """
    if os.name != "nt":
        raise OSError(
            "JPEG resizing through GDI+ requires Windows."
        )

    gdiplus = ctypes.WinDLL("gdiplus.dll")

    # Configure the GDI+ API signatures.
    gdiplus.GdiplusStartup.argtypes = [
        ctypes.POINTER(ctypes.c_size_t),
        ctypes.POINTER(GdiplusStartupInput),
        ctypes.c_void_p,
    ]
    gdiplus.GdiplusStartup.restype = ctypes.c_int

    gdiplus.GdiplusShutdown.argtypes = [ctypes.c_size_t]
    gdiplus.GdiplusShutdown.restype = None

    gdiplus.GdipLoadImageFromFile.argtypes = [
        ctypes.c_wchar_p,
        ctypes.POINTER(ctypes.c_void_p),
    ]
    gdiplus.GdipLoadImageFromFile.restype = ctypes.c_int

    gdiplus.GdipGetImageWidth.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_uint32),
    ]
    gdiplus.GdipGetImageWidth.restype = ctypes.c_int

    gdiplus.GdipGetImageHeight.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_uint32),
    ]
    gdiplus.GdipGetImageHeight.restype = ctypes.c_int

    gdiplus.GdipCreateBitmapFromScan0.argtypes = [
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_void_p),
    ]
    gdiplus.GdipCreateBitmapFromScan0.restype = ctypes.c_int

    gdiplus.GdipGetImageGraphicsContext.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_void_p),
    ]
    gdiplus.GdipGetImageGraphicsContext.restype = ctypes.c_int

    gdiplus.GdipSetInterpolationMode.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
    ]
    gdiplus.GdipSetInterpolationMode.restype = ctypes.c_int

    gdiplus.GdipDrawImageRectI.argtypes = [
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
    ]
    gdiplus.GdipDrawImageRectI.restype = ctypes.c_int

    gdiplus.GdipSaveImageToFile.argtypes = [
        ctypes.c_void_p,
        ctypes.c_wchar_p,
        ctypes.POINTER(GUID),
        ctypes.POINTER(EncoderParameters),
    ]
    gdiplus.GdipSaveImageToFile.restype = ctypes.c_int

    gdiplus.GdipDeleteGraphics.argtypes = [ctypes.c_void_p]
    gdiplus.GdipDeleteGraphics.restype = ctypes.c_int

    gdiplus.GdipDisposeImage.argtypes = [ctypes.c_void_p]
    gdiplus.GdipDisposeImage.restype = ctypes.c_int

    # JPEG encoder CLSID.
    jpeg_encoder = GUID(
        0x557CF401,
        0x1A04,
        0x11D3,
        (ctypes.c_ubyte * 8)(
            0x9A, 0x73, 0x00, 0x00,
            0xF8, 0x1E, 0xF3, 0x2E
        ),
    )

    # EncoderQuality GUID.
    quality_guid = GUID(
        0x1D5BE4B5,
        0xFA4A,
        0x452D,
        (ctypes.c_ubyte * 8)(
            0x9C, 0xDD, 0x5D, 0xB3,
            0x51, 0x05, 0xE7, 0xEB
        ),
    )

    token = ctypes.c_size_t()
    startup_input = GdiplusStartupInput(
        1,      # GDI+ version
        None,   # Debug callback
        0,      # Do not suppress background thread
        0,      # Do not suppress external codecs
    )

    source = ctypes.c_void_p()
    destination = ctypes.c_void_p()
    graphics = ctypes.c_void_p()

    temp_path = None
    started = False
    saved = False

    try:
        status = gdiplus.GdiplusStartup(
            ctypes.byref(token),
            ctypes.byref(startup_input),
            None,
        )
        check_gdiplus(status, "GDI+ initialization")
        started = True

        # Load the source JPEG.
        status = gdiplus.GdipLoadImageFromFile(
            str(path),
            ctypes.byref(source),
        )
        check_gdiplus(status, f"Loading {path.name}")

        width = ctypes.c_uint32()
        height = ctypes.c_uint32()

        check_gdiplus(
            gdiplus.GdipGetImageWidth(
                source, ctypes.byref(width)
            ),
            "Reading image width",
        )

        check_gdiplus(
            gdiplus.GdipGetImageHeight(
                source, ctypes.byref(height)
            ),
            "Reading image height",
        )

        new_width = max(1, int(width.value * scale))
        new_height = max(1, int(height.value * scale))

        # Create an RGB destination bitmap.
        check_gdiplus(
            gdiplus.GdipCreateBitmapFromScan0(
                new_width,
                new_height,
                0,
                PIXEL_FORMAT_24BPP_RGB,
                None,
                ctypes.byref(destination),
            ),
            "Creating destination bitmap",
        )

        check_gdiplus(
            gdiplus.GdipGetImageGraphicsContext(
                destination,
                ctypes.byref(graphics),
            ),
            "Creating graphics context",
        )

        check_gdiplus(
            gdiplus.GdipSetInterpolationMode(
                graphics,
                INTERPOLATION_HIGH_QUALITY_BILINEAR,
            ),
            "Setting interpolation mode",
        )

        # Resize the source into the destination bitmap.
        check_gdiplus(
            gdiplus.GdipDrawImageRectI(
                graphics,
                source,
                0,
                0,
                new_width,
                new_height,
            ),
            "Resizing JPEG",
        )

        # Create a unique temporary filename.
        # GDI+ will create the output file itself.
        fd, temp_name = tempfile.mkstemp(
            prefix=f".{path.stem}_resize_",
            suffix=path.suffix,
            dir=str(path.parent),
        )
        os.close(fd)

        temp_path = Path(temp_name)
        temp_path.unlink()

        # Configure JPEG quality.
        quality = ctypes.c_uint32(95)

        parameters = EncoderParameters()
        parameters.Count = 1
        parameters.Parameter[0].Guid = quality_guid
        parameters.Parameter[0].NumberOfValues = 1
        parameters.Parameter[0].Type = ENCODER_PARAMETER_TYPE_LONG
        parameters.Parameter[0].Value = ctypes.cast(
            ctypes.pointer(quality),
            ctypes.c_void_p,
        )

        # Save explicitly as JPEG, regardless of the filename suffix.
        check_gdiplus(
            gdiplus.GdipSaveImageToFile(
                destination,
                str(temp_path),
                ctypes.byref(jpeg_encoder),
                ctypes.byref(parameters),
            ),
            "Saving resized JPEG",
        )
        saved = True

    finally:
        # Release GDI+ resources before replacing the source file.
        if graphics.value:
            gdiplus.GdipDeleteGraphics(graphics)

        if destination.value:
            gdiplus.GdipDisposeImage(destination)

        if source.value:
            gdiplus.GdipDisposeImage(source)

        if started:
            gdiplus.GdiplusShutdown(token.value)

        # Remove incomplete output files on failure.
        if temp_path is not None and not saved:
            try:
                temp_path.unlink(missing_ok=True)
            except OSError:
                pass

    # Only replace the source after a successful save and cleanup.
    try:
        os.replace(temp_path, path)
    except Exception:
        try:
            temp_path.unlink(missing_ok=True)
        except OSError:
            pass
        raise


def bilinear_resize(img, scale):
    """Resize a Tkinter PhotoImage using bilinear interpolation."""
    src_w = img.width()
    src_h = img.height()

    new_w = max(1, int(src_w * scale))
    new_h = max(1, int(src_h * scale))

    out = tk.PhotoImage(width=new_w, height=new_h)

    for y in range(new_h):
        for x in range(new_w):
            gx = x / scale
            gy = y / scale

            x0 = int(math.floor(gx))
            y0 = int(math.floor(gy))

            x1 = min(x0 + 1, src_w - 1)
            y1 = min(y0 + 1, src_h - 1)

            dx = gx - x0
            dy = gy - y0

            c00 = img.get(x0, y0)
            c10 = img.get(x1, y0)
            c01 = img.get(x0, y1)
            c11 = img.get(x1, y1)

            r = (
                c00[0] * (1 - dx) * (1 - dy)
                + c10[0] * dx * (1 - dy)
                + c01[0] * (1 - dx) * dy
                + c11[0] * dx * dy
            )

            g = (
                c00[1] * (1 - dx) * (1 - dy)
                + c10[1] * dx * (1 - dy)
                + c01[1] * (1 - dx) * dy
                + c11[1] * dx * dy
            )

            b = (
                c00[2] * (1 - dx) * (1 - dy)
                + c10[2] * dx * (1 - dy)
                + c01[2] * (1 - dx) * dy
                + c11[2] * dx * dy
            )

            color = f"#{int(r):02x}{int(g):02x}{int(b):02x}"
            out.put(color, (x, y))

    return out


def resize_tk_image_to_png(path, scale):
    """
    Resize a non-JPEG image using Tkinter and save it as PNG.

    Existing behavior is retained: non-PNG supported formats are
    converted to PNG. Existing PNG files are replaced safely.
    """
    img = tk.PhotoImage(file=str(path))
    resized = bilinear_resize(img, scale)

    # Keep an existing PNG's exact filename.
    output_path = (
        path if path.suffix.lower() == ".png"
        else path.with_suffix(".png")
    )

    fd, temp_name = tempfile.mkstemp(
        prefix=f".{path.stem}_resize_",
        suffix=".png",
        dir=str(path.parent),
    )
    os.close(fd)

    temp_path = Path(temp_name)

    try:
        # Tkinter writes to the temporary PNG.
        resized.write(str(temp_path), format="png")

        # Avoid accidentally overwriting a different existing PNG.
        if output_path != path and output_path.exists():
            raise FileExistsError(
                f"Cannot convert {path.name}: "
                f"{output_path.name} already exists."
            )

        os.replace(temp_path, output_path)

        # Remove the original only after the output has been saved.
        if output_path != path:
            path.unlink()

    finally:
        try:
            temp_path.unlink(missing_ok=True)
        except OSError:
            pass


def main():
    try:
        factor = float(
            input("Enter scale factor (example 0.5): ").strip()
        )

        if not 0 < factor < 1:
            print("Use a value between 0 and 1.")
            return

    except ValueError:
        print("Invalid number.")
        return

    folder = Path(__file__).resolve().parent

    root = tk.Tk()
    root.withdraw()

    processed = 0
    skipped = 0

    # Snapshot the directory so temporary outputs are not processed.
    files = list(folder.iterdir())

    for path in files:
        if not path.is_file():
            continue

        if path.suffix.lower() not in SUPPORTED_EXTS:
            continue

        try:
            print(f"Processing {path.name}...")

            if path.suffix.lower() in JPEG_EXTS:
                # JPEGs are loaded and resized using Windows GDI+.
                resize_jpeg_gdiplus(path, factor)
            else:
                # Other formats retain the existing Tkinter path.
                resize_tk_image_to_png(path, factor)

            processed += 1

        except Exception as e:
            print(f"Skipped {path.name}: {e}")
            skipped += 1

    root.destroy()

    print("\nDone.")
    print(f"Processed: {processed}")
    print(f"Skipped: {skipped}")


if __name__ == "__main__":
    main()