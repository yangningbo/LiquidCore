//
// FileSystem.java
//
// LiquidPlayer project
// https://github.com/LiquidPlayer
//
// Created by Eric Lange
//
/*
 Copyright (c) 2016 Eric Lange. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
package org.liquidplayer.node;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.support.v4.content.ContextCompat;

import org.liquidplayer.javascript.JSContext;
import org.liquidplayer.javascript.JSFunction;
import org.liquidplayer.javascript.JSObject;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * This creates a JavaScript object that is used by nodedroid_file.cc in the native code
 * to control access to the file system.  We will create an alias filesystem with the
 * following structure.
 *
 * home
 *  |
 *  +--- modules
 *  |
 *  +--- temp
 *  |
 *  +--- cache
 *  |
 *  +--- local
 *  |
 *  +--- public
 *         |
 *         +--- data
 *         |
 *         +--- media
 *                |
 *                +--- Pictures
 *                |
 *                +--- Movies
 *                |
 *                +--- Ringtones
 *                |
 *                +--- ...
 *
 * /home
 *
 * Read-only directory which acts only as a holding bin for other directories
 *
 * /home/modules
 *
 * Read-only directory.  Contains downloaded javascript modules for the given MicroService.  As
 * these files change on the server, they will be updated and cached here.
 *
 * /home/temp
 *
 * Read-write directory.  This directory is private to a single instance of the MicroService.
 * The contents of this directory live only as long as the underlying Node.js process is active.
 * This directory is appropriate only for very short-lived temporary files.
 *
 * /home/cache
 *
 * Read-write directory.  This directory is available to all instances of the same MicroService
 * running on the same local host.  The host, in this case, is the host app which uses an instance
 * of the LiquidCore library.  Files in this directory will be deleted on an as-needed basis
 * as they age and/or a memory space quota is breaching.
 *
 * /home/local
 *
 * Read-write directory.  This directory is the persistent storage for a MicroService and is shared
 * amongst all instances of the same MicroService running on the same local host.  All content in
 * this directory will persist so long as the MicroService is present on the host.  This directory
 * will only be cleared when the MicroService is "uninstalled".  Uninstallation happens for
 * MicroServices that have not been used in a long time and when space is required for installing
 * new MicroServices.
 *
 * /home/public
 *
 * Read-only directory which acts as a holding bin for other public directories.  This directory
 * may not exist if no external (sdcard) storage is available.
 *
 * /home/public/data
 *
 * Read-write directory for MicroService-specific data.  This directory is shared between all
 * instances of a MicroService on all hosts (local or not).  Its contents are publicly available for
 * any app to access, though its true location on external media is a bit obscured.  This directory
 * persists so long as a MicroService exists on any host.  If a MicroService is uninstalled from
 * every host, this directory will also be cleared.
 *
 * /home/public/media
 *
 * Read-only holding directory for public media-specific directories.
 *
 * /home/public/media/[MediaType]
 *
 * Read or read-write directory (depending on permissions given by the host) for known media types.
 * These types include Pictures, Movies, Ringtones, Music, Downloads, etc. as exposed by Android and
 * typically reside at a true location like /sdcard/Pictures, for example.  Files in these
 * directories are never cleared by LiquidCore, but can be managed by any other app or service.
 *
 * Everything else will result in a ENOACES (access denied) error
 */
class FileSystem extends JSObject {

    @jsexport(attributes = JSPropertyAttributeReadOnly) @SuppressWarnings("unused")
    private Property<JSObject> access_;

    @jsexport(attributes = JSPropertyAttributeReadOnly) @SuppressWarnings("unused")
    private Property<JSObject> aliases_;

    @jsexport(attributes = JSPropertyAttributeReadOnly) @SuppressWarnings("unused")
    private Property<JSFunction> fs;

    @jsexport(attributes = JSPropertyAttributeReadOnly) @SuppressWarnings("unused")
    private Property<JSFunction> alias;

    @jsexport  @SuppressWarnings("unused")
    private Property<String> cwd;

    private final Context androidCtx;
    private String uniqueID;

    private String realDir(String dir) {
        return getContext().evaluateScript(
                "(function(){return require('fs').realpathSync('" + dir + "');})()"
        ).toString();
    }
    private String mkdir(String dir) {
        if (new File(dir).mkdirs()) {
            android.util.Log.i("mkdir", "Created directory " + dir);
        }
        return realDir(dir);
    }
    private List<String> toclean = new ArrayList<>();
    private void symlink(String target, String linkpath) {
        getContext().evaluateScript(
                "(function(){require('fs').symlinkSync('" + target + "','" + linkpath +"');})()"
        );
    }
    private boolean isSymlink(File file) {
        try {
            return file.getCanonicalFile().equals(file.getAbsoluteFile());
        } catch (IOException e) {
            return true;
        }
    }
    private void deleteRecursive(File fileOrDirectory) {
        if (fileOrDirectory.isDirectory() && !isSymlink(fileOrDirectory))
            for (File child : fileOrDirectory.listFiles())
                deleteRecursive(child);

        if (!fileOrDirectory.delete()) {
            android.util.Log.d("nodedroid", "Failed to delete directory");
        }
    }
    private void linkMedia(String type, String dir, String home, int mediaPermissionsMask) {
        File external = Environment.getExternalStoragePublicDirectory(type);
        if (external.mkdirs()) {
            android.util.Log.i("linkMedia", "Created external directory " + external);
        }
        String media = realDir(external.getAbsolutePath());
        symlink(media, home + "/public/media/" + dir);
        aliases_.get().property("/home/public/media/" + dir, media);
        access_.get().property("/home/public/media/" + dir, mediaPermissionsMask);
    }

    private void setUp(int mediaPermissionsMask) {
        final String suffix = "/__org.liquidplayer.node__/_" + uniqueID;
        String random = "" + UUID.randomUUID();
        String sessionSuffix = suffix + "/" + random;

        // Set up /home (read-only)
        String home  = mkdir(androidCtx.getFilesDir().getAbsolutePath() +
                sessionSuffix + "/home");
        toclean.add(home);
        aliases_.get().property("/home", home);
        access_ .get().property("/home", Process.kMediaAccessPermissionsRead);

        // Set up /home/modules (read-only)
        String modules = mkdir(androidCtx.getFilesDir().getAbsolutePath() +
                suffix + "/modules");
        symlink(modules, home + "/modules");
        aliases_.get().property("/home/modules", modules);
        access_ .get().property("/home/modules", Process.kMediaAccessPermissionsRead);

        // Set up /home/temp (read/write)
        String temp = mkdir(androidCtx.getCacheDir().getAbsolutePath() +
                sessionSuffix + "/temp");
        toclean.add(temp);
        symlink(temp, home + "/temp");
        aliases_.get().property("/home/temp", temp);
        access_ .get().property("/home/temp", Process.kMediaAccessPermissionsRW);

        // Set up /home/cache (read/write)
        String cache = mkdir(androidCtx.getCacheDir().getAbsolutePath() +
                suffix + "/cache");
        symlink(cache, home + "/cache");
        aliases_.get().property("/home/cache", cache);
        access_ .get().property("/home/cache", Process.kMediaAccessPermissionsRW);

        // Set up /home/local (read/write)
        String local = mkdir(androidCtx.getFilesDir().getAbsolutePath() +
                suffix + "/local");
        symlink(local, home + "/local");
        aliases_.get().property("/home/local", local);
        access_ .get().property("/home/local", Process.kMediaAccessPermissionsRW);

        String state = Environment.getExternalStorageState();
        if (!Environment.MEDIA_MOUNTED.equals(state)){
            android.util.Log.w("FileSystem", "Warning: external storage is unavailable");
        } else {
            if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state) &&
                    (mediaPermissionsMask & Process.kMediaAccessPermissionsWrite) != 0) {
                android.util.Log.w("FileSystem", "Warning: external storage is read only.");
                mediaPermissionsMask &= ~Process.kMediaAccessPermissionsWrite;
            }

            // Set up /home/public
            if (!new File(home + "/public").mkdirs()) {
                android.util.Log.e("FileSystem", "Error: Failed to set up /home/public");
            }

            // Set up /home/public/data
            File external = androidCtx.getExternalFilesDir(null);
            if (external != null) {
                String externalPersistent = mkdir(external.getAbsolutePath() +
                        "/LiquidPlayer/" + uniqueID);
                symlink(externalPersistent, home + "/public/data");
                aliases_.get().property("/home/public/data", externalPersistent);
                access_.get().property("/home/public/data", Process.kMediaAccessPermissionsRW);
            }

            // Set up /home/public/media
            if (!new File(home + "/public/media").mkdirs()) {
                android.util.Log.e("FileSystem", "Error: Failed to set up /home/public/media");
            }

            if ((mediaPermissionsMask & Process.kMediaAccessPermissionsWrite) != 0 &&
                    ContextCompat.checkSelfPermission(androidCtx,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED) {

                mediaPermissionsMask &= ~Process.kMediaAccessPermissionsWrite;
            }

            linkMedia(Environment.DIRECTORY_MOVIES,   "Movies",   home, mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_PICTURES, "Pictures", home, mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_DCIM,     "DCIM",     home, mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_ALARMS,   "Alarms",   home, mediaPermissionsMask);
            if (Build.VERSION.SDK_INT >= 19) {
                linkMedia(Environment.DIRECTORY_DOCUMENTS, "Documents", home, mediaPermissionsMask);
            }
            linkMedia(Environment.DIRECTORY_DOWNLOADS,"Downloads",home, mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_MUSIC,    "Music"    ,home, mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_NOTIFICATIONS, "Notifications", home,
                    mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_PODCASTS, "Podcasts", home, mediaPermissionsMask);
            linkMedia(Environment.DIRECTORY_RINGTONES,"Ringtones",home, mediaPermissionsMask);
        }

        cwd.set("/home");
    }

    private void tearDown() {
        for (String dir : toclean) {
            deleteRecursive(new File(dir));
        }
    }

    FileSystem(JSContext ctx, Context androidCtx,
               String uniqueID, int mediaPermissionsMask) {
        super(ctx);
        this.androidCtx = androidCtx;
        this.uniqueID = uniqueID;

        aliases_.set(new JSObject(ctx));
        access_ .set(new JSObject(ctx));

        setUp(mediaPermissionsMask);

        fs.set(new JSFunction(ctx, "fs", ""+
                "if (!file.startsWith('/')) { file = this.cwd+'/'+file; }" +
                "try { file = require('path').resolve(file); } catch (e) {}"+
                "var access = 0;"+
                "for (var p in this.aliases_) {"+
                "    if (file.startsWith(this.aliases_[p] + '/')) {"+
                "        file = p + '/' + file.substring(this.aliases_[p].length + 1);"+
                "        break;"+
                "    } else if (file == this.aliases_[p]) {"+
                "        file = p;"+
                "        break;"+
                "    }"+
                "}"+
                "var keys = Object.keys(this.access_).sort().reverse();"+
                "for (var p=0; p<keys.length; p++) {"+
                "    if (file.startsWith(keys[p] + '/') || keys[p]==file) {"+
                "        access = this.access_[keys[p]];"+
                "        break;"+
                "    }"+
                "}"+
                "var newfile = file;"+
                "for (var p in this.aliases_) {"+
                "    if (file.startsWith(p + '/')) {"+
                "        newfile = this.aliases_[p] + '/' + file.substring(p.length + 1);"+
                "        break;"+
                "    } else if (file == p) {"+
                "        newfile = this.aliases_[p];"+
                "        break;"+
                "    }"+
                "}"+
                "return [access,newfile];",
                "file"));

        alias.set(new JSFunction(ctx, "alias", ""+
                "for (var p in this.aliases_) {"+
                "   if (file.startsWith(this.aliases_[p] + '/')) {"+
                "       file = p + '/' + file.substring(this.aliases_[p].length + 1);"+
                "       break;"+
                "   } else if (file == this.aliases_[p]) {"+
                "       file = p;"+
                "       break;"+
                "   }"+
                "}"+
                "return file;",
                "file"));

    }

    void cleanUp() {
        tearDown();
    }
}
