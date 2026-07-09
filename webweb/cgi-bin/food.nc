#include <neo-c.h>
#include <neo-c-net.h>
#include <time.h>
#include <stdint.h>

int main(int argc, char** argv)
{
    buffer*% input_buf = stdin.fread()!;
    string input = input_buf.to_string();
    
    char *cookie = getenv("HTTP_COOKIE");
    
    char username[1024] = {0};
    if (cookie) {
        sscanf(cookie, "username=%1023[^;]", username);
    }

    for(int i = 0; username[i] != '\0'; i++) {
        if(username[i] == '\'' || username[i] == '\\' || username[i] == '"') {
            username[0] = '\0';
            break;
        }
    }

    if(username[0] != '\0') {
        const char *query1 = "CREATE DATABASE testdb";
        client_socket2(port:3366, query1)!;
        
        const char *query2 = "use testdb";
        client_socket2(port:3366, query2)!;
        
        const char *query3 = "CREATE TABLE IF NOT EXISTS food ("
                             "id INT AUTO_INCREMENT PRIMARY KEY, "
                             "username VARCHAR(100) NOT NULL, "
                             "time VARCHAR(100) NOT NULL"
                             ")";
        client_socket2(port:3366, query3)!;
        
        string query4 = s"SELECT MAX(time) FROM food WHERE username = '\{username}'";
        string read_data = client_socket2(port:3366, query4)!;
        
        long time_ = 0L;
        (void)sscanf(read_data, "%ld", &time_);
        
        if(time_ == 0L) {
            long time_value = time(NULL);
            string time2 = xsprintf("%ld", time_value);
            
            string query = s"INSERT INTO food(username, time) VALUES('\{username}', '\{time2}')";
            client_socket2(port:3366, query)!;
            puts("""
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0,maximum-scale=1, user-scalable=no">
    <title>メインページ</title>
    <style>
        a {
            touch-action: manipulation;
            text-decoration: none;
            color: blue;
        }
    </style>
</head>
<body>
<p>エサを食べています</p>

<img src="/cgi-bin/images/food2.jpeg" alt="title" style="width: 30%; height: 30%;">

<a href="/cgi-bin/main.cgi" rel="nofollow">戻る</a>


</body>
</html>
            """);
        }
        else {
            long time_value = time(NULL);
            
            if((time_value - time_) > 60*60*3) {
                string time_str2 = xsprintf("%ld", time_value);
                
                string query = s"INSERT INTO food(username, time) VALUES('\{username}', '\{time_str2}')";
                client_socket2(port:3366, query)!;
                puts("""
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0,maximum-scale=1, user-scalable=no">
    <title>メインページ</title>
    <style>
        a {
            touch-action: manipulation;
            text-decoration: none;
            color: blue;
        }
    </style>
</head>
<body>
<p>エサを食べています</p>

<img src="/cgi-bin/images/food2.jpeg" alt="title" style="width: 30%; height: 30%;">

<a href="/cgi-bin/main.cgi" rel="nofollow">戻る</a>


</body>
</html>
                """);
            }
            else {
                puts("""
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0,maximum-scale=1, user-scalable=no">
    <title>メインページ</title>
    <style>
        a {
            touch-action: manipulation;
            text-decoration: none;
            color: blue;
        }
    </style>
</head>
<body>
<p>お腹いっぱいな様子です。 </p>

<a href="/cgi-bin/main.cgi" rel="nofollow">戻る</a>

</body>
</html>
                """);
            }
        }
    }
    else {
        puts("""
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0,maximum-scale=1, user-scalable=no">
    <title>ログインが必要です</title>
</head>
<body>
<p>ログインが必要です。</p>
<a href="/index.html" rel="nofollow">戻る</a>
</body>
</html>
        """);
    }

    return 0;
}
