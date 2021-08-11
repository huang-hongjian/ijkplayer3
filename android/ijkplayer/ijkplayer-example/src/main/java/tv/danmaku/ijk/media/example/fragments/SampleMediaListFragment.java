/*
 * Copyright (C) 2015 Bilibili
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package tv.danmaku.ijk.media.example.fragments;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import tv.danmaku.ijk.media.example.R;
import tv.danmaku.ijk.media.example.activities.VideoActivity;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;

import com.google.gson.JsonArray;
import com.google.gson.JsonIOException;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonSyntaxException;

public class SampleMediaListFragment extends Fragment {
    private static String TAG="SampleMediaListFragment";
    private ListView mFileListView;
    private SampleMediaAdapter mAdapter;
    private String url_str[] = {""};

    public Handler uiHandler=new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            switch (msg.what){
                case 0: {
                    /*
                    String str = url_str;
                    DocumentBuilderFactory bdf = DocumentBuilderFactory.newInstance();
                    DocumentBuilder db = null;
                    Document document = null;
                    try {
                        db = bdf.newDocumentBuilder();
                        document = db.parse(new ByteArrayInputStream(str.getBytes()));
                        NodeList list = document.getElementsByTagName("video");
                        for (int i = 0; i < list.getLength(); i++) {
                            Element element = (Element)list.item(i);
                            String url = element.getElementsByTagName("url").item(0).getFirstChild().getNodeValue();
                            System.out.println(url);
                            Log.d(TAG,"url="+url);
                            String name = element.getElementsByTagName("name").item(0).getFirstChild().getNodeValue();
                            Log.d(TAG,"name="+name);
                            if(url!=null && !url.equals("") &&  name!=null && !name.equals(""))
                            {
                                Log.d(TAG,"Adapter.addItem   name="+name+"  url"+url);
                                mAdapter.addItem(url,name);
                            }
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                */
                    JSONObject jobj = null;
                    JSONObject jsonVideo = null;
                    String line = null;

                    String ChannelName = null;
                    String ChannelURL = null;
                    String key = null;
                    String value = null;
                    BufferedReader br = new BufferedReader(new InputStreamReader(new ByteArrayInputStream(url_str[0].getBytes(Charset.forName("utf8"))), Charset.forName("utf8")));

                    try {

                        while ( (line = br.readLine()) != null ) {
                            if(!line.trim().equals("")){

                                jobj = new JSONObject(line);
                                String method = jobj.optString("method");
                                JSONObject parameters = jobj.optJSONObject("parameters");
                                key = parameters.optString("key");
                                value = parameters.optString("value");
                                jsonVideo = new JSONObject("{" + value + "}");
                                //String id = data.optString("UserChannelID");
                                //Log.e(TAG,line);
                                ChannelName = jsonVideo.optString("ChannelName");
                                ChannelURL = jsonVideo.optString("ChannelURL");
                                //Log.e(TAG,ChannelName +"\n"+ChannelURL);
                                mAdapter.addItem(ChannelURL,ChannelName);
                            }
                        }

                       // Log.e("json——->", id + "," + title + "," + pic + "," + collect_num + "," + food_str + "," + num);
                    }
                    catch (JSONException e) {
                        e.printStackTrace();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    //setRecycleView(list);
                    //initLayout("normal");
                }
                    break;
                default:
                    break;
            }
            return false;
        }
    });

    public static SampleMediaListFragment newInstance() {
        SampleMediaListFragment f = new SampleMediaListFragment();
        return f;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        ViewGroup viewGroup = (ViewGroup) inflater.inflate(R.layout.fragment_file_list, container, false);
        mFileListView = (ListView) viewGroup.findViewById(R.id.file_list_view);
        return viewGroup;
    }

    public  String convertStreamToString(InputStream is) {
        BufferedReader reader = new BufferedReader(new InputStreamReader(is));
        StringBuilder sb = new StringBuilder();

        String line = null;
        try {
            while ((line = reader.readLine()) != null) {
                sb.append(line + "\n");
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return sb.toString();
    }
    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        final Activity activity = getActivity();

        mAdapter = new SampleMediaAdapter(activity);
        mFileListView.setAdapter(mAdapter);
        mFileListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, final int position, final long id) {
                SampleMediaItem item = mAdapter.getItem(position);
                String name = item.mName;
                String url = item.mUrl;
                VideoActivity.intentTo(activity, url, name);
            }
        });
        /*
        <?xml version="1.0" encoding="UTF-8"?>
        <user>
            <video>
                <url>http://xxxxxxx</url>
                <name>xxxxx</name>
            </video>
            <video>
                <url>http://xxxxxxxxxx</url>
                <name>xxxx</name>
            </video>
        </user>
        */

            final HttpClient httpclient = new DefaultHttpClient();
            final String[] str = {""};
            // 创建Get方法实例
            final HttpGet httpgets = new HttpGet("http://huanghongjian.xyz:8080/iptvepg/video.xml");

            new Thread(new Runnable(){
                @Override
                public void run() {
                    try {
                        Log.d(TAG,"httpclient---》");
                        HttpResponse response = httpclient.execute(httpgets);
                        HttpEntity entity = response.getEntity();
                        if (entity != null) {
                            InputStream instreams = entity.getContent();
                            url_str[0] = convertStreamToString(instreams);
                            Log.d(TAG,""+ url_str[0]);
                            // Do not need the rest
                            httpgets.abort();
                            uiHandler.sendEmptyMessage(0);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    Log.d(TAG,"httpclient---》》");
                }
            }).start();




        //Document document = db.parse(new File("user.xml"));

        //mAdapter.addItem("rtsp://111.48.16.199:1554/TVOD/88888888/224/3221225684/10000100000000060000000000252976_0.smil?playseek=20181206150100-20181206155100&foreign=10000100000000070000000021056535&rrsip=111.48.16.199:1554&icpid=SSPID&accounttype=1&limitflux=-1&limitdur=-1&GuardEncType=2&accountinfo=xNY%2FG%2B9qlYBLkmSnL%2BZSJRQovAnjsODD8MfPqZ%2BLtd48WWH1tNz4PZNWwznde7JeIH91hQSp9lyOvP4Y92RRdI9LBEHDVhfm4XBNbjScFiQHJWEjCYQKzctPcsebhKKFL5T%2B8wY50iY28eg7pgYJElYWLEqSG1YW64DeI8YTJmgiQ0OQiNxq9x2fhJ1zbMry%3A20181206161538%2Ctest2016111006%2C120.198.34.172%2C20181206161538%2C00000001000000070000000058437478%2C60E33940DBD1784F3A5E9E68173DA9C3%2C-1%2C1%2C300%2C%2C%2C4%2C0%2C0%2C%2C5%2CEND","回看CCTV1");
        mAdapter.addItem("http://huanghongjian.xyz:8080/iptvepg/jay.ts", "jay.ts");
        mAdapter.addItem("rtsp://111.48.16.199:1554/88888888/16/20180810/269495219/269495219.ts?rrsip=111.48.16.199:1554&starttime=0&icpid=SSPID&accounttype=1&limitflux=-1&limitdur=-1&GuardEncType=2&accountinfo=xNY%2FG%2B9qlYBLkmSnL%2BZSJRQovAnjsODD8MfPqZ%2BLtd48WWH1tNz4PZNWwznde7JeIH91hQSp9lyOvP4Y92RRdI9LBEHDVhfm4XBNbjScFiRcghV6p7QOV1mbcfELFUzXBiE96eWV0vvEx6rPVPFgLg%3D%3D%3A20181206161010%2Ctest2016111006%2C120.198.34.172%2C20181206161010%2C74dc7651c9494d498455a2582e702080%2C60E33940DBD1784F3A5E9E68173DA9C3%2C0%2C1%2C0%2C%2C%2C6%2C0%2C0%2C%2C1%2CEND","特种部队 4K");
        mAdapter.addItem("rtsp://111.48.16.199:1554/88888888/16/20180810/269495219/269495219.ts?rrsip=111.48.16.199:1554&starttime=0&icpid=SSPID&accounttype=1&limitflux=-1&limitdur=-1&GuardEncType=2&accountinfo=xNY%2FG%2B9qlYBLkmSnL%2BZSJRQovAnjsODD8MfPqZ%2BLtd48WWH1tNz4PZNWwznde7JeIH91hQSp9lyOvP4Y92RRdI9LBEHDVhfm4XBNbjScFiRcghV6p7QOV1mbcfELFUzXBiE96eWV0vvEx6rPVPFgLg%3D%3D%3A20181206161010%2Ctest2016111006%2C120.198.34.172%2C20181206161010%2C74dc7651c9494d498455a2582e702080%2C60E33940DBD1784F3A5E9E68173DA9C3%2C0%2C1%2C0%2C%2C%2C6%2C0%2C0%2C%2C1%2CEND", "书写人生4K");
        mAdapter.addItem("rtsp://111.48.16.199:1554/PLTV/88888888/224/3221225673/10000100000000060000000000246403_0.smil?rrsip=111.48.16.199:1554&icpid=SSPID&accounttype=1&limitflux=-1&limitdur=-1&GuardEncType=2&accountinfo=xNY%2FG%2B9qlYBLkmSnL%2BZSJRQovAnjsODD8MfPqZ%2BLtd48WWH1tNz4PZNWwznde7JeIH91hQSp9lyOvP4Y92RRdI9LBEHDVhfm4XBNbjScFiQ1SxQjqedU1ZDnLsYLB5Y2tgEeFFD5L3Mdlo4AAAxl4w%3D%3D%3A20181206160453%2Ctest2016111006%2C120.198.34.172%2C20181206160453%2C99180001000000050000000000000222%2C60E33940DBD1784F3A5E9E68173DA9C3%2C-1%2C0%2C1%2C%2C%2C2%2C%2C%2C%2C2%2CEND", "CCTV5");
        mAdapter.addItem("rtsp://111.48.16.199:1554/PLTV/88888888/224/3221225585/10000100000000060000000000246489_0.smil?rrsip=111.48.16.199:1554&icpid=SSPID&accounttype=1&limitflux=-1&limitdur=-1&GuardEncType=2&accountinfo=xNY%2FG%2B9qlYBLkmSnL%2BZSJRQovAnjsODD8MfPqZ%2BLtd48WWH1tNz4PZNWwznde7JeIH91hQSp9lyOvP4Y92RRdI9LBEHDVhfm4XBNbjScFiQ1SxQjqedU1ZDnLsYLB5Y2oP2k6XB2hYz%2FujMcD1xnUA%3D%3D%3A20181206160453%2Ctest2016111006%2C120.198.34.172%2C20181206160453%2C00000001000000050000000000000058%2C60E33940DBD1784F3A5E9E68173DA9C3%2C-1%2C0%2C1%2C%2C%2C2%2C%2C%2C%2C2%2CEND", "CCTV2");        
        mAdapter.addItem("rtsp://223.75.40.74:1554/PLTV/88888888/224/3221225684/10000100000000060000000000252976_0.smil?rrsip=111.48.16.199:1554&icpid=SSPID&accounttype=1&limitflux=-1&limitdur=-1&GuardEncType=2&accountinfo=xNY%2FG%2B9qlYBLkmSnL%2BZSJRQovAnjsODD8MfPqZ%2BLtd48WWH1tNz4PZNWwznde7JeIH91hQSp9lyOvP4Y92RRdI9LBEHDVhfm4XBNbjScFiQ1SxQjqedU1ZDnLsYLB5Y2ILUrPTqsF102OtZMAOu9iQ%3D%3D%3A20181206160453%2Ctest2016111006%2C120.198.34.172%2C20181206160453%2C00000001000000050000000000000152%2C60E33940DBD1784F3A5E9E68173DA9C3%2C-1%2C0%2C1%2C%2C%2C2%2C%2C%2C%2C2%2CEND&from=2&hms_devid=24","CCTV1 HD");
        mAdapter.addItem("http://huanghongjian.xyz:8080/iptvepg/jay.ts", "jay.ts");
        mAdapter.addItem("http://66.98.125.144:8080/iptvepg/jay.ts", "jay.ts");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/bipbop_4x3_variant.m3u8", "bipbop basic master playlist");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/prog_index.m3u8", "bipbop basic 400x300 @ 232 kbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8", "bipbop basic 640x480 @ 650 kbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear3/prog_index.m3u8", "bipbop basic 640x480 @ 1 Mbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear4/prog_index.m3u8", "bipbop basic 960x720 @ 2 Mbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear0/prog_index.m3u8", "bipbop basic 22.050Hz stereo @ 40 kbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/bipbop_16x9_variant.m3u8", "bipbop advanced master playlist");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear1/prog_index.m3u8", "bipbop advanced 416x234 @ 265 kbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear2/prog_index.m3u8", "bipbop advanced 640x360 @ 580 kbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear3/prog_index.m3u8", "bipbop advanced 960x540 @ 910 kbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear4/prog_index.m3u8", "bipbop advanced 1289x720 @ 1 Mbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear5/prog_index.m3u8", "bipbop advanced 1920x1080 @ 2 Mbps");
        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear0/prog_index.m3u8", "bipbop advanced 22.050Hz stereo @ 40 kbps");
    }

    final class SampleMediaItem {
        String mUrl;
        String mName;

        public SampleMediaItem(String url, String name) {
            mUrl = url;
            mName = name;
        }
    }

    final class SampleMediaAdapter extends ArrayAdapter<SampleMediaItem> {
        public SampleMediaAdapter(Context context) {
            super(context, android.R.layout.simple_list_item_2);
        }

        public void addItem(String url, String name) {
            add(new SampleMediaItem(url, name));
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                LayoutInflater inflater = LayoutInflater.from(parent.getContext());
                view = inflater.inflate(android.R.layout.simple_list_item_2, parent, false);
            }

            ViewHolder viewHolder = (ViewHolder) view.getTag();
            if (viewHolder == null) {
                viewHolder = new ViewHolder();
                viewHolder.mNameTextView = (TextView) view.findViewById(android.R.id.text1);
                viewHolder.mUrlTextView = (TextView) view.findViewById(android.R.id.text2);
            }

            SampleMediaItem item = getItem(position);
            viewHolder.mNameTextView.setText(item.mName);
            viewHolder.mUrlTextView.setText(item.mUrl);

            return view;
        }

        final class ViewHolder {
            public TextView mNameTextView;
            public TextView mUrlTextView;
        }
    }
}
