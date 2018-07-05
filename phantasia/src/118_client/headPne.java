import java.awt.*;

public class headPne extends Canvas implements constants {

    String title1;
    String title2;
    int cx = 0;
    int canvasWidth = 0;
    int canvasHeight = 0;
    Rectangle theBounds = null;
    FontMetrics fm = null;
    Font theFont = new Font("TimesRoman", Font.BOLD, 14);
    Font theFont2 = new Font("Helvetica", Font.PLAIN, 12);

    public headPne(String message) {

	super();
	title1 = message;
	title2 = "";
	setBackground(highlightColor);
	setSize(38, 38);
    }

    public void paint(Graphics g) {

	theBounds = getBounds();
	canvasWidth = theBounds.width - 1;
	canvasHeight = theBounds.height - 1;

	g.setColor(highlightColor);
	g.clearRect(0, 0, canvasWidth, canvasHeight);

	fm = getFontMetrics(theFont);
	cx = (int) ((canvasWidth - fm.stringWidth(title1)) / 2);
	g.setColor(foregroundColor);

	g.setFont(theFont);
	g.drawString(title1, cx, 16);

	fm = getFontMetrics(theFont2);
	cx = (int) ((canvasWidth - fm.stringWidth(title2)) / 2);

	g.setFont(theFont2);
	g.drawString(title2, cx, canvasHeight - 6);
    }

    public void changeHead(String message) {

	title1 = message;
	paint(getGraphics());
    }

    public void changeTail(String message) {
	title2 = message;
	paint(getGraphics());
    }
}
