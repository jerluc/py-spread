<?
/*
if(!extension_loaded('spread')) {
	dl('spread.' . PHP_SHLIB_SUFFIX);
        echo "dl ok!\n";
}
*/
echo "SPREAD_VERSION: " . SPREAD_VERSION . "\n";
echo "SP_MAX_GROUP: " . SP_MAX_GROUP . "\n";
$module = 'spread';
$functions = get_extension_funcs($module);
echo "Functions available in the spread extension:<br>\n";
foreach($functions as $func) {
    echo $func."<br>\n";
}
echo "<br>\n";

$id = spread_connect("3333@127.0.0.1", "phptest");
echo "spread connect:";
echo $id ? "True" : "False";
echo "\n";

var_dump($id);

$group = array("test", "ddddd", "sssss");
$ret = spread_join($group);
echo "spread join: ";
echo $ret ? "True" : "false";
echo "\n";
var_dump($ret);

$ret = spread_multicast("test", "123456789");
echo "spread_multicast: ";
var_dump($ret);

$msg = spread_receive($id, 5);
//echo "received message " . $msg['message'] . "\n";
var_dump($msg);
$group[3] = "dsdsds";
$ret = spread_leave($group);
echo "spread leave: ";
echo $ret ? "True" : "False";
echo "\n";

$flag = spread_close();
echo "close: ";
echo ($flag) ? "True" : "False";
echo "\n";

$errno = spread_errno();
echo "spread errno: " . $errno . "\n";

$error = spread_error();
echo "spread error: " . $error . "\n";
var_dump($error);
?>
